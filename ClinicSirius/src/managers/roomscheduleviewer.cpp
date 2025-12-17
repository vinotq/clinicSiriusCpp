#include "managers/roomscheduleviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <algorithm>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QMessageBox>
#include <QMenu>
#include "patients/appointmentbookingwidget.h"

RoomScheduleViewer::RoomScheduleViewer(QWidget *parent)
    : QWidget(parent), m_dataManager(QString()) {
    m_startDate = getMondayOfWeek(QDate::currentDate());
    buildUI();
    loadRooms();
}

void RoomScheduleViewer::buildUI() {
    setWindowTitle("Расписание кабинетов");
    resize(900, 600);

    QVBoxLayout *main = new QVBoxLayout(this);

    // Room filter with completer and interval control
    QHBoxLayout *top = new QHBoxLayout();
    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("Поиск кабинета...");
    top->addWidget(m_filterEdit, 1);
    
    // Add interval spinner
    top->addSpacing(16);
    QLabel* intervalLabel = new QLabel("Интервал (мин):");
    m_intervalSpin = new QSpinBox();
    m_intervalSpin->setRange(5, 120);
    m_intervalSpin->setValue(20);
    m_intervalSpin->setFixedWidth(70);
    top->addWidget(intervalLabel);
    top->addWidget(m_intervalSpin);
    
    main->addLayout(top);

    // Week navigation
    QHBoxLayout *nav = new QHBoxLayout();
    m_prevBtn = new QPushButton("←");
    m_prevBtn->setFixedWidth(40);
    m_nextBtn = new QPushButton("→");
    m_nextBtn->setFixedWidth(40);
    m_weekLabel = new QLabel();
    m_weekLabel->setAlignment(Qt::AlignCenter);
    m_todayBtn = new QPushButton("Сегодня");
    m_todayBtn->setFixedWidth(80);
    nav->addWidget(m_prevBtn);
    nav->addStretch();
    nav->addWidget(m_weekLabel, 0);
    nav->addStretch();
    nav->addWidget(m_todayBtn);
    nav->addWidget(m_nextBtn);
    main->addLayout(nav);

    m_scheduleTable = new QTableWidget();
    // Prevent in-place editing
    m_scheduleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_scheduleTable->setColumnCount(8);
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Set minimum row height to prevent text truncation
    m_scheduleTable->verticalHeader()->setDefaultSectionSize(35);
    m_scheduleTable->verticalHeader()->setVisible(false);
    m_scheduleTable->setContextMenuPolicy(Qt::CustomContextMenu);
    main->addWidget(m_scheduleTable, 1);

    connect(m_filterEdit, &QLineEdit::textChanged, this, &RoomScheduleViewer::onRoomFilterChanged);
    connect(m_prevBtn, &QPushButton::clicked, this, &RoomScheduleViewer::onPrevWeek);
    connect(m_nextBtn, &QPushButton::clicked, this, &RoomScheduleViewer::onNextWeek);
    connect(m_todayBtn, &QPushButton::clicked, this, &RoomScheduleViewer::onToday);
    connect(m_intervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &RoomScheduleViewer::onIntervalChanged);
    connect(m_scheduleTable, &QTableWidget::customContextMenuRequested, this, &RoomScheduleViewer::onTableContextMenu);
    
    // Cell click to open booking widget
    connect(m_scheduleTable, &QTableWidget::cellClicked, this, [this](int row, int column){
        if (column == 0) return; // time column
        QTableWidgetItem *it = m_scheduleTable->item(row, column);
        int probe = row;
        while (!it && probe >= 0) { --probe; it = m_scheduleTable->item(probe, column); }
        if (!it) return;
        int schId = it->data(Qt::UserRole).toInt();
        if (schId <= 0) return;
        AppointmentSchedule sch = m_dataManager.getScheduleById(schId);
        if (sch.id_ap_sch <= 0) return;

        // Check if slot is booked and show options
        QString status = sch.status.trimmed().toLower();
        if (status == "booked" || status == "busy") {
            // Find the appointment for detailed info
            Appointment apt;
            bool foundAppointment = false;
            for (const Appointment &a : m_dataManager.getAllAppointments()) {
                if (a.id_ap_sch == schId) {
                    apt = a;
                    foundAppointment = true;
                    break;
                }
            }
            
            // Build detail message
            QString detailMsg = "Этот слот занят.";
            if (foundAppointment) {
                Patient patient = m_dataManager.getPatientById(apt.id_patient);
                detailMsg = QString("Слот занят пациентом: %1\n\nВремя: %2 - %3")
                    .arg(patient.fullName())
                    .arg(sch.time_from.toString("HH:mm"))
                    .arg(sch.time_to.toString("HH:mm"));
            }
            
            // Show options - Reschedule or Cancel
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Слот занят");
            msgBox.setText(detailMsg);
            msgBox.setIcon(QMessageBox::Information);
            
            QPushButton *rescheduleBtn = msgBox.addButton("⏩ Перенести", QMessageBox::ActionRole);
            QPushButton *cancelBtn = msgBox.addButton("Отменить", QMessageBox::DestructiveRole);
            QPushButton *closeBtn = msgBox.addButton("Закрыть", QMessageBox::RejectRole);
            
            msgBox.setDefaultButton(closeBtn);
            msgBox.exec();
            
            if (msgBox.clickedButton() == cancelBtn) {
                // Show confirmation for cancellation
                QMessageBox::StandardButton confirmReply = QMessageBox::question(this, 
                    "Подтверждение отмены", 
                    "Вы уверены, что хотите отменить эту запись?",
                    QMessageBox::Yes | QMessageBox::No);
                
                if (confirmReply == QMessageBox::Yes) {
                    // Delete appointment and mark slot as free
                    if (foundAppointment) {
                        m_dataManager.deleteAppointment(apt.id_ap);
                    }
                    
                    AppointmentSchedule updatedSch = sch;
                    updatedSch.status = "free";
                    m_dataManager.updateSchedule(updatedSch);
                    
                    // Reload schedule
                    loadScheduleForRoom(m_currentRoomId);
                    QMessageBox::information(this, "Успешно", "Запись отменена");
                }
            } else if (msgBox.clickedButton() == rescheduleBtn) {
                // Open reschedule widget for editing appointment
                if (foundAppointment && apt.id_ap > 0) {
                    AppointmentBookingWidget *reschedule = new AppointmentBookingWidget();
                    reschedule->setAttribute(Qt::WA_DeleteOnClose);
                    LoginUser managerUser(LoginUser::MANAGER, -1, "Менеджер");
                    reschedule->setUser(managerUser);
                    reschedule->setRescheduleMode(apt.id_ap, sch.id_ap_sch);
                    reschedule->setWindowTitle("Перенос приема");
                    reschedule->resize(900, 700);
                    reschedule->show();
                    
                    // Reload schedule when reschedule widget closes
                    connect(reschedule, &QObject::destroyed, this, [this](QObject*){
                        loadScheduleForRoom(m_currentRoomId);
                    });
                }
            }
            
            return;
        }

        // Open booking widget directly (no extra modal wrapper)
        AppointmentBookingWidget *booking = new AppointmentBookingWidget();
        booking->setAttribute(Qt::WA_DeleteOnClose);
        LoginUser managerUser(LoginUser::MANAGER, -1, "Менеджер");
        booking->setUser(managerUser);
        booking->setInitialSelection(sch.id_doctor, sch.id_ap_sch);
        booking->setWindowTitle("Запись на слот — менеджер");
        booking->resize(900, 700);
        booking->show();

        // When the booking widget closes, reload schedules in case it was booked
        connect(booking, &QObject::destroyed, this, [this](QObject*){
            loadScheduleForRoom(m_currentRoomId);
        });
    });
}

void RoomScheduleViewer::loadRooms() {
    m_allRooms = m_dataManager.getAllRooms();
    // Sort rooms by room number
    std::sort(m_allRooms.begin(), m_allRooms.end(), [](const Room &a, const Room &b){
        return a.room_number.toInt() < b.room_number.toInt();
    });
    
    // Set up completer with room numbers
    QStringList roomNumbers;
    for (const Room &r : m_allRooms) {
        roomNumbers.append(r.room_number);
    }
    roomNumbers.sort();
    m_roomCompleter = new QCompleter(roomNumbers, this);
    m_roomCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_filterEdit->setCompleter(m_roomCompleter);
    
    m_currentRoomId = -1;
    // Don't load schedule by default - table should be empty until user selects a room
}

void RoomScheduleViewer::onRoomFilterChanged(const QString &text) {
    // Apply room filter in real-time
    applyRoomFilter(text);
}

void RoomScheduleViewer::applyRoomFilter(const QString &text) {
    QString trimmedText = text.trimmed();
    
    if (trimmedText.isEmpty()) {
        m_currentRoomId = -1;
    } else {
        m_currentRoomId = -1;
        for (const Room &r : m_allRooms) {
            if (r.room_number.contains(trimmedText, Qt::CaseInsensitive)) {
                m_currentRoomId = r.id_room;
                break;
            }
        }
    }
    loadScheduleForRoom(m_currentRoomId);
}

void RoomScheduleViewer::onPrevWeek() {
    m_startDate = m_startDate.addDays(-7);
    loadScheduleForRoom(m_currentRoomId);
}

void RoomScheduleViewer::onNextWeek() {
    m_startDate = m_startDate.addDays(7);
    loadScheduleForRoom(m_currentRoomId);
}

void RoomScheduleViewer::onToday() {
    m_startDate = getMondayOfWeek(QDate::currentDate());
    loadScheduleForRoom(m_currentRoomId);
}

void RoomScheduleViewer::onIntervalChanged(int value) {
    m_timeIntervalMinutes = value;
    loadScheduleForRoom(m_currentRoomId);
}

void RoomScheduleViewer::loadScheduleForRoom(int roomId) {
    m_scheduleTable->clear();
    m_scheduleTable->setRowCount(0);
    m_scheduleTable->setColumnCount(1);

    // If no room is selected, show empty table with message
    if (roomId <= 0) {
        QStringList headers;
        headers << "Выберите кабинет";
        m_scheduleTable->setColumnCount(1);
        m_scheduleTable->setHorizontalHeaderLabels(headers);
        
        QTableWidgetItem *msgItem = new QTableWidgetItem("Выберите кабинет для просмотра расписания");
        msgItem->setTextAlignment(Qt::AlignCenter);
        msgItem->setFlags(msgItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        m_scheduleTable->setRowCount(1);
        m_scheduleTable->setItem(0, 0, msgItem);
        return;
    }

    QStringList headers;
    headers << "Время";
    QDate start = m_startDate;
    QDate today = QDate::currentDate();
    int todayColumn = -1;  // Column for today's date
    
    if (m_weekLabel) {
        QString label = QString("%1 — %2").arg(start.toString("dd.MM.yyyy"), start.addDays(6).toString("dd.MM.yyyy"));
        m_weekLabel->setText(label);
    }
    for (int i = 0; i < 7; ++i) {
        QDate dt = start.addDays(i);
        if (dt == today) {
            todayColumn = i + 1;  // +1 because column 0 is time
        }
        QString dayName;
        switch (dt.dayOfWeek()) {
            case 1: dayName = "Пн"; break;
            case 2: dayName = "Вт"; break;
            case 3: dayName = "Ср"; break;
            case 4: dayName = "Чт"; break;
            case 5: dayName = "Пт"; break;
            case 6: dayName = "Сб"; break;
            case 7: dayName = "Вс"; break;
            default: dayName = ""; break;
        }
        headers << QString("%1\n%2").arg(dt.toString("dd.MM"), dayName);
    }
    m_scheduleTable->setColumnCount(headers.size());
    m_scheduleTable->setHorizontalHeaderLabels(headers);
    
    // Highlight today's column if visible
    if (todayColumn > 0 && todayColumn < m_scheduleTable->columnCount()) {
        QTableWidgetItem *headerItem = m_scheduleTable->horizontalHeaderItem(todayColumn);
        if (headerItem) {
            headerItem->setBackground(QColor(200, 220, 255));  // Light blue background for today
            headerItem->setFont([](){ QFont f; f.setBold(true); return f; }());
        }
    }

    // For room view: collect schedules for this room
    QList<AppointmentSchedule> schedules = m_dataManager.getSchedulesByRoom(roomId);

    // Use m_timeIntervalMinutes instead of calculating GCD
    int minimalInterval = m_timeIntervalMinutes;
    if (minimalInterval < 5) minimalInterval = 5;
    if (minimalInterval > 120) minimalInterval = 120;

    const int dayStartMin = 6 * 60;
    const int dayEndMin = 22 * 60;
    int totalMinutes = dayEndMin - dayStartMin;
    int rowsCount = (totalMinutes + minimalInterval - 1) / minimalInterval;
    m_scheduleTable->setRowCount(rowsCount);

    for (int r = 0; r < rowsCount; ++r) {
        int minutes = dayStartMin + r * minimalInterval;
        int hour = minutes / 60;
        int minute = minutes % 60;
        QTableWidgetItem *timeItem = new QTableWidgetItem(QString("%1:%2").arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0')));
        // Make time column non-editable and non-selectable
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        m_scheduleTable->setItem(r, 0, timeItem);
    }

    for (const AppointmentSchedule &s : schedules) {
        int dayOffset = m_startDate.daysTo(s.time_from.date());
        int column = 1 + dayOffset;
        if (column < 1 || column >= m_scheduleTable->columnCount()) continue;

        int startMinOfDay = s.time_from.time().hour() * 60 + s.time_from.time().minute();
        int endMinOfDay = s.time_to.time().hour() * 60 + s.time_to.time().minute();
        if (startMinOfDay < dayStartMin || endMinOfDay > dayEndMin) continue;

        int startIndex = (startMinOfDay - dayStartMin) / minimalInterval;
        int durationMin = endMinOfDay - startMinOfDay;
        int rowSpan = (durationMin + minimalInterval - 1) / minimalInterval;
        if (rowSpan <= 0) rowSpan = 1;

        QString st = s.status.trimmed().toLower();
        QString statusText = "Свободен";
        QColor bgColor = QColor(144, 190, 109);
        QString tooltipText = "";
        
        if (st == "booked" || st == "busy") {
            statusText = "Занято";
            bgColor = QColor(255, 165, 0);
            
            // Get appointment info for tooltip
            Appointment apt;
            bool found = false;
            for (const Appointment &a : m_dataManager.getAllAppointments()) {
                if (a.id_ap_sch == s.id_ap_sch) {
                    apt = a;
                    found = true;
                    break;
                }
            }
            if (found) {
                Patient patient = m_dataManager.getPatientById(apt.id_patient);
                Doctor doctor = m_dataManager.getDoctorById(apt.id_doctor);
                tooltipText = QString("ID записи: %1\nПациент: %2\nВрач: %3\nВремя: %4 - %5\nСтатус: %6")
                    .arg(apt.id_ap)
                    .arg(patient.fullName())
                    .arg(doctor.fullName())
                    .arg(s.time_from.toString("HH:mm"))
                    .arg(s.time_to.toString("HH:mm"))
                    .arg(statusText);
            }
        }

        QTableWidgetItem *cell = new QTableWidgetItem(statusText);
        // Make schedule cells non-editable
        cell->setFlags(cell->flags() & ~Qt::ItemIsEditable);
        cell->setBackground(bgColor);
        cell->setForeground(Qt::white);
        cell->setTextAlignment(Qt::AlignCenter);
        cell->setData(Qt::UserRole, s.id_ap_sch);
        
        // Slightly adjust background color for today's slots if visible
        if (column == todayColumn && todayColumn > 0) {
            // Make color slightly darker/brighter to indicate today
            bgColor = bgColor.darker(110);
            cell->setBackground(bgColor);
        }
        
        // Add tooltip if appointment has info
        if (!tooltipText.isEmpty()) {
            cell->setToolTip(tooltipText);
        }
        
        m_scheduleTable->setItem(startIndex, column, cell);
        if (rowSpan > 1) {
            m_scheduleTable->setSpan(startIndex, column, rowSpan, 1);
        }
    }
    
    // Highlight empty cells in today's column with light background
    // Make all empty free slots non-selectable/non-clickable
    if (todayColumn > 0) {
        for (int r = 0; r < rowsCount; ++r) {
            QTableWidgetItem *cell = m_scheduleTable->item(r, todayColumn);
            if (!cell) {
                // Create empty cell for today's column with light background
                QTableWidgetItem *emptyCell = new QTableWidgetItem("");
                emptyCell->setBackground(QColor(230, 240, 255));  // Very light blue
                emptyCell->setFlags(emptyCell->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
                emptyCell->setToolTip("Слот свободен");
                m_scheduleTable->setItem(r, todayColumn, emptyCell);
            }
        }
    }
    
    // Make all remaining empty cells non-selectable
    for (int r = 0; r < rowsCount; ++r) {
        for (int c = 1; c < m_scheduleTable->columnCount(); ++c) {
            QTableWidgetItem *cell = m_scheduleTable->item(r, c);
            if (!cell) {
                // Create empty cell for free slots - non-selectable
                QTableWidgetItem *emptyCell = new QTableWidgetItem("");
                emptyCell->setBackground(QColor(240, 240, 240));  // Very light gray
                emptyCell->setFlags(emptyCell->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
                emptyCell->setToolTip("Слот свободен");
                m_scheduleTable->setItem(r, c, emptyCell);
            }
        }
    }
}

void RoomScheduleViewer::onTableContextMenu(const QPoint &pos) {
    QTableWidgetItem *item = m_scheduleTable->itemAt(pos);
    if (!item) return;
    
    int schId = item->data(Qt::UserRole).toInt();
    if (schId <= 0) return;
    
    AppointmentSchedule sch = m_dataManager.getScheduleById(schId);
    if (sch.status.toLower() != "booked" && sch.status.toLower() != "busy") {
        return; // Only show menu for booked slots
    }
    
    QMenu contextMenu;
    QAction *cancelAction = contextMenu.addAction("Отменить запись");
    
    QAction *selectedAction = contextMenu.exec(m_scheduleTable->mapToGlobal(pos));
    
    if (selectedAction == cancelAction) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "Подтверждение", 
            "Вы уверены, что хотите отменить эту запись?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            // Get appointment and delete it
            for (const Appointment &apt : m_dataManager.getAllAppointments()) {
                if (apt.id_ap_sch == schId) {
                    m_dataManager.deleteAppointment(apt.id_ap);
                }
            }
            
            // Mark slot as free
            AppointmentSchedule updatedSch = sch;
            updatedSch.status = "free";
            m_dataManager.updateSchedule(updatedSch);
            
            // Reload schedule
            loadScheduleForRoom(m_currentRoomId);
            QMessageBox::information(this, "Успешно", "Запись отменена");
        }
    }
}

QDate RoomScheduleViewer::getMondayOfWeek(const QDate &date) const {
    // Qt's dayOfWeek: 1 = Monday, 7 = Sunday
    int daysFromMonday = date.dayOfWeek() - 1;
    return date.addDays(-daysFromMonday);
}
