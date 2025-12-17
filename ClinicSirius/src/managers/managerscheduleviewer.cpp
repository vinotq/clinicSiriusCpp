#include "managers/managerscheduleviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <algorithm>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QIcon>
#include <QMessageBox>
#include <QMenu>
#include <QSize>
#include "patients/appointmentbookingwidget.h"

ManagerScheduleViewer::ManagerScheduleViewer(QWidget *parent)
    : QWidget(parent), m_dataManager(nullptr) {
    // fallback: caller should set DataManager via constructor overload or externally
    m_startDate = getMondayOfWeek(QDate::currentDate());
    buildUI();
    loadDoctors();
}

ManagerScheduleViewer::ManagerScheduleViewer(DataManager *dm, QWidget *parent)
    : QWidget(parent), m_dataManager(dm) {
    m_startDate = getMondayOfWeek(QDate::currentDate());
    buildUI();
    loadDoctors();
}

void ManagerScheduleViewer::buildUI() {
    setWindowTitle("Расписание врачей");
    resize(900, 600);

    QVBoxLayout *main = new QVBoxLayout(this);

    // Doctor filter with completer and interval control
    QHBoxLayout *top = new QHBoxLayout();
    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("Поиск врача...");
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
    m_prevBtn = new QPushButton();
    m_prevBtn->setIcon(QIcon(":/images/icon-arrow-left.svg"));
    m_prevBtn->setIconSize(QSize(16, 16));
    m_prevBtn->setFixedWidth(40);
    m_prevBtn->setToolTip("Предыдущая неделя");
    m_nextBtn = new QPushButton();
    m_nextBtn->setIcon(QIcon(":/images/icon-arrow-right.svg"));
    m_nextBtn->setIconSize(QSize(16, 16));
    m_nextBtn->setFixedWidth(40);
    m_nextBtn->setToolTip("Следующая неделя");
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

    connect(m_filterEdit, &QLineEdit::textChanged, this, &ManagerScheduleViewer::onDoctorFilterChanged);
    connect(m_prevBtn, &QPushButton::clicked, this, &ManagerScheduleViewer::onPrevWeek);
    connect(m_nextBtn, &QPushButton::clicked, this, &ManagerScheduleViewer::onNextWeek);
    connect(m_todayBtn, &QPushButton::clicked, this, &ManagerScheduleViewer::onToday);
    connect(m_intervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ManagerScheduleViewer::onIntervalChanged);
    connect(m_scheduleTable, &QTableWidget::customContextMenuRequested, this, &ManagerScheduleViewer::onTableContextMenu);
    
    // Cell click to open booking widget
    connect(m_scheduleTable, &QTableWidget::cellClicked, this, [this](int row, int column){
        if (column == 0) return; // time column
        QTableWidgetItem *it = m_scheduleTable->item(row, column);
        int probe = row;
        while (!it && probe >= 0) { --probe; it = m_scheduleTable->item(probe, column); }
        if (!it) return;
        int schId = it->data(Qt::UserRole).toInt();
        if (schId <= 0) return;
        if (!m_dataManager) return;
        AppointmentSchedule sch = m_dataManager->getScheduleById(schId);
        if (sch.id_ap_sch <= 0) return;

        // Check if slot is booked and show options
        QString status = sch.status.trimmed().toLower();
        if (status == "booked" || status == "busy") {
            // Find the appointment for detailed info
            Appointment apt;
            bool foundAppointment = false;
            for (const Appointment &a : m_dataManager->getAllAppointments()) {
                if (a.id_ap_sch == schId) {
                    apt = a;
                    foundAppointment = true;
                    break;
                }
            }
            
            // Build detail message
            QString detailMsg = "Этот слот занят.";
            if (foundAppointment) {
                Patient patient = m_dataManager->getPatientById(apt.id_patient);
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
            
            QPushButton *rescheduleBtn = new QPushButton(QIcon(":/images/icon-refresh.svg"), "Перенести");
            msgBox.addButton(rescheduleBtn, QMessageBox::ActionRole);
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
                        m_dataManager->deleteAppointment(apt.id_ap);
                    }
                    
                    AppointmentSchedule updatedSch = sch;
                    updatedSch.status = "free";
                    m_dataManager->updateSchedule(updatedSch);
                    
                    // Reload schedule
                    loadScheduleForDoctor(m_currentDoctorId);
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
                        loadScheduleForDoctor(m_currentDoctorId);
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
            loadScheduleForDoctor(m_currentDoctorId);
        });
    });
}

void ManagerScheduleViewer::loadDoctors() {
    if (m_dataManager) m_allDoctors = m_dataManager->getAllDoctors();
    // Sort doctors by full name
    std::sort(m_allDoctors.begin(), m_allDoctors.end(), [](const Doctor &a, const Doctor &b){
        return a.fullName() < b.fullName();
    });
    
    // Set up completer with doctor names
    QStringList doctorNames;
    for (const Doctor &d : m_allDoctors) {
        doctorNames.append(d.fullName());
    }
    doctorNames.sort();
    m_doctorCompleter = new QCompleter(doctorNames, this);
    m_doctorCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_filterEdit->setCompleter(m_doctorCompleter);
    
    m_currentDoctorId = -1;
    // Don't load schedule by default - table should be empty until user selects a doctor
}

void ManagerScheduleViewer::onDoctorFilterChanged(const QString &text) {
    // Apply doctor filter in real-time
    applyDoctorFilter(text);
}

void ManagerScheduleViewer::applyDoctorFilter(const QString &text) {
    QString trimmedText = text.trimmed();
    
    if (trimmedText.isEmpty()) {
        m_currentDoctorId = -1;
    } else {
        m_currentDoctorId = -1;
        for (const Doctor &d : m_allDoctors) {
            if (d.fullName().contains(trimmedText, Qt::CaseInsensitive)) {
                m_currentDoctorId = d.id_doctor;
                break;
            }
        }
    }
    loadScheduleForDoctor(m_currentDoctorId);
}

void ManagerScheduleViewer::onPrevWeek() {
    m_startDate = m_startDate.addDays(-7);
    loadScheduleForDoctor(m_currentDoctorId);
}

void ManagerScheduleViewer::onNextWeek() {
    m_startDate = m_startDate.addDays(7);
    loadScheduleForDoctor(m_currentDoctorId);
}

void ManagerScheduleViewer::onToday() {
    m_startDate = getMondayOfWeek(QDate::currentDate());
    loadScheduleForDoctor(m_currentDoctorId);
}

void ManagerScheduleViewer::onIntervalChanged(int value) {
    m_timeIntervalMinutes = value;
    loadScheduleForDoctor(m_currentDoctorId);
}

void ManagerScheduleViewer::loadScheduleForDoctor(int doctorId) {
    m_scheduleTable->clear();
    m_scheduleTable->setRowCount(0);
    m_scheduleTable->setColumnCount(1);

    // If no doctor is selected, show empty table with message
    if (doctorId <= 0) {
        QStringList headers;
        headers << "Выберите врача";
        m_scheduleTable->setColumnCount(1);
        m_scheduleTable->setHorizontalHeaderLabels(headers);
        
        QTableWidgetItem *msgItem = new QTableWidgetItem("Выберите врача для просмотра расписания");
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
    int todayColumn = -1;
    
    if (m_weekLabel) {
        QString label = QString("%1 — %2").arg(start.toString("dd.MM.yyyy"), start.addDays(6).toString("dd.MM.yyyy"));
        m_weekLabel->setText(label);
    }
    for (int i = 0; i < 7; ++i) {
        QDate dt = start.addDays(i);
        if (dt == today) {
            todayColumn = i + 1;
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
            headerItem->setBackground(QColor(200, 220, 255));
            headerItem->setFont([](){ QFont f; f.setBold(true); return f; }());
        }
    }

    // For doctor view: collect schedules for this doctor
    if (!m_dataManager) return;
    QList<AppointmentSchedule> schedules = m_dataManager->getDoctorSchedules(doctorId);

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
            
            Appointment apt;
            bool found = false;
            for (const Appointment &a : m_dataManager->getAllAppointments()) {
                if (a.id_ap_sch == s.id_ap_sch) {
                    apt = a;
                    found = true;
                    break;
                }
            }
            if (found) {
                Patient patient = m_dataManager->getPatientById(apt.id_patient);
                Room room = m_dataManager->getRoomById(s.id_room);
                tooltipText = QString("ID записи: %1\nПациент: %2\nКабинет: %3\nВремя: %4 - %5\nСтатус: %6")
                    .arg(apt.id_ap)
                    .arg(patient.fullName())
                    .arg(room.room_number)
                    .arg(s.time_from.toString("HH:mm"))
                    .arg(s.time_to.toString("HH:mm"))
                    .arg(statusText);
            }
        }

        QTableWidgetItem *cell = new QTableWidgetItem(statusText);
        cell->setFlags(cell->flags() & ~Qt::ItemIsEditable);
        cell->setBackground(bgColor);
        cell->setForeground(Qt::white);
        cell->setTextAlignment(Qt::AlignCenter);
        cell->setData(Qt::UserRole, s.id_ap_sch);
        
        if (column == todayColumn && todayColumn > 0) {
            bgColor = bgColor.darker(110);
            cell->setBackground(bgColor);
        }
        
        if (!tooltipText.isEmpty()) {
            cell->setToolTip(tooltipText);
        }
        
        m_scheduleTable->setItem(startIndex, column, cell);
        if (rowSpan > 1) {
            m_scheduleTable->setSpan(startIndex, column, rowSpan, 1);
        }
    }
    
    if (todayColumn > 0) {
        for (int r = 0; r < rowsCount; ++r) {
            QTableWidgetItem *cell = m_scheduleTable->item(r, todayColumn);
            if (!cell) {
                QTableWidgetItem *emptyCell = new QTableWidgetItem("");
                emptyCell->setBackground(QColor(230, 240, 255));
                emptyCell->setFlags(emptyCell->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
                emptyCell->setToolTip("Слот свободен");
                m_scheduleTable->setItem(r, todayColumn, emptyCell);
            }
        }
    }
    
    for (int r = 0; r < rowsCount; ++r) {
        for (int c = 1; c < m_scheduleTable->columnCount(); ++c) {
            QTableWidgetItem *cell = m_scheduleTable->item(r, c);
            if (!cell) {
                QTableWidgetItem *emptyCell = new QTableWidgetItem("");
                emptyCell->setBackground(QColor(240, 240, 240));
                emptyCell->setFlags(emptyCell->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
                emptyCell->setToolTip("Слот свободен");
                m_scheduleTable->setItem(r, c, emptyCell);
            }
        }
    }
}

void ManagerScheduleViewer::onTableContextMenu(const QPoint &pos) {
    QTableWidgetItem *item = m_scheduleTable->itemAt(pos);
    if (!item) return;
    
    int schId = item->data(Qt::UserRole).toInt();
    if (schId <= 0) return;
    
    if (!m_dataManager) return;
    AppointmentSchedule sch = m_dataManager->getScheduleById(schId);
    if (sch.status.toLower() != "booked" && sch.status.toLower() != "busy") {
        return;
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
            for (const Appointment &apt : m_dataManager->getAllAppointments()) {
                if (apt.id_ap_sch == schId) {
                    m_dataManager->deleteAppointment(apt.id_ap);
                }
            }
            
            AppointmentSchedule updatedSch = sch;
            updatedSch.status = "free";
            m_dataManager->updateSchedule(updatedSch);
            
            loadScheduleForDoctor(m_currentDoctorId);
            QMessageBox::information(this, "Успешно", "Запись отменена");
        }
    }
}

QDate ManagerScheduleViewer::getMondayOfWeek(const QDate &date) const {
    int daysFromMonday = date.dayOfWeek() - 1;
    return date.addDays(-daysFromMonday);
}

void ManagerScheduleViewer::setCurrentDoctor(int doctorId) {
    m_currentDoctorId = doctorId;
    loadScheduleForDoctor(doctorId);
}
