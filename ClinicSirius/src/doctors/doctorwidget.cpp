#include "doctorwidget.h"
#include "doctorvisitdialog.h"
#include "addslotdialog.h"
#include <QHeaderView>
#include <QDate>
#include <QDateTime>
#include <QCoreApplication>
#include <QTableWidgetItem>
#include <QMap>
#include <QColor>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QStackedWidget>
#include <QSpinBox>
#include <numeric>

DoctorWidget::DoctorWidget(QWidget *parent)
    : QWidget(parent), dataManager(QCoreApplication::applicationDirPath() + "/../data") {
    scheduleStartDate = QDate::currentDate();
    buildUI();
}

void DoctorWidget::buildUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    stackedWidget = new QStackedWidget(this);
    
    buildMainPage();
    buildSchedulePage();
    
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);
}

void DoctorWidget::buildMainPage() {
    QWidget *mainPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(mainPage);
    
    // Приветствие
    mainTitleLabel = new QLabel();
    mainTitleLabel->setProperty("class", "main-title");
    QFont welcomeFont;
    welcomeFont.setPointSize(24);
    welcomeFont.setBold(true);
    mainTitleLabel->setFont(welcomeFont);
    layout->addWidget(mainTitleLabel);
    
    // Кнопки на весь функционал
    viewScheduleButton = new QPushButton("📅 Посмотреть расписание");
    viewScheduleButton->setIconSize(QSize(18,18));
    viewScheduleButton->setMinimumHeight(60);
    layout->addWidget(viewScheduleButton);
    
    addSlotButton = new QPushButton("➕ Добавить окно для приема");
    addSlotButton->setIconSize(QSize(16,16));
    addSlotButton->setMinimumHeight(60);
    layout->addWidget(addSlotButton);
    
    bookAppointmentButton = new QPushButton("📝 Записать пациента на прием");
    bookAppointmentButton->setIconSize(QSize(16,16));
    bookAppointmentButton->setMinimumHeight(60);
    layout->addWidget(bookAppointmentButton);
    
    layout->addStretch();
    
    mainPageIndex = stackedWidget->addWidget(mainPage);
    
    connect(viewScheduleButton, &QPushButton::clicked, this, &DoctorWidget::onViewSchedule);
    connect(addSlotButton, &QPushButton::clicked, this, &DoctorWidget::onAddSlot);
    connect(bookAppointmentButton, &QPushButton::clicked, this, &DoctorWidget::onBookAppointment);
}

void DoctorWidget::buildSchedulePage() {
    QWidget *schedulePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(schedulePage);
    
    scheduleTitleLabel = new QLabel("Расписание");
    scheduleTitleLabel->setProperty("class", "main-title");
    layout->addWidget(scheduleTitleLabel);

    // Week navigation (previous / week label / next)
    QHBoxLayout *weekNav = new QHBoxLayout();
    prevWeekButton = new QPushButton();
    prevWeekButton->setMaximumWidth(40);
    prevWeekButton->setText("←");
    prevWeekButton->setIconSize(QSize(16,16));
    weekLabel = new QLabel();
    weekLabel->setAlignment(Qt::AlignCenter);
    QFont wl; wl.setBold(true); wl.setPointSize(11); weekLabel->setFont(wl);
    nextWeekButton = new QPushButton("→");
    nextWeekButton->setMaximumWidth(40);
    todayButton = new QPushButton("Сегодня");
    todayButton->setMaximumWidth(90);
    weekNav->addWidget(prevWeekButton);
    weekNav->addStretch();
    weekNav->addWidget(weekLabel);
    weekNav->addWidget(todayButton);
    weekNav->addStretch();
    weekNav->addWidget(nextWeekButton);
    layout->addLayout(weekNav);
    
    // Time slot duration input (for grid granularity in schedule)
    QHBoxLayout *timeSlotLayout = new QHBoxLayout();
    timeSlotLayout->addStretch();
    QLabel *timeSlotLabel = new QLabel("⏱️ Время приема (мин):");
    timeSlotDurationSpinBox = new QSpinBox();
    timeSlotDurationSpinBox->setMinimum(5);
    timeSlotDurationSpinBox->setMaximum(120);
    timeSlotDurationSpinBox->setValue(20);
    timeSlotDurationSpinBox->setSuffix(" мин");
    timeSlotDurationSpinBox->setMaximumWidth(100);
    selectedIntervalMinutes = 20;
    timeSlotLayout->addWidget(timeSlotLabel);
    timeSlotLayout->addWidget(timeSlotDurationSpinBox);
    layout->addLayout(timeSlotLayout);
    
    scheduleTable = new QTableWidget();
    scheduleTable->setColumnCount(8);
    scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    scheduleTable->verticalHeader()->setVisible(false);
    scheduleTable->setEditTriggers(QTableWidget::NoEditTriggers);
    scheduleTable->setSelectionMode(QTableWidget::SingleSelection);
    layout->addWidget(scheduleTable);
    
    // Кнопки действий
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    bookFromScheduleButton = new QPushButton("📝 Записать на выбранный слот");
    bookFromScheduleButton->setIconSize(QSize(16,16));
    deleteSlotButton = new QPushButton("🗑 Удалить слот");
    deleteSlotButton->setIconSize(QSize(16,16));
    // Новая кнопка
    addSlotInScheduleButton = new QPushButton("➕ Добавить окно для приема");
    addSlotInScheduleButton->setIconSize(QSize(16,16));
    backButton = new QPushButton("Назад");
    backButton->setText("← " + backButton->text());
    backButton->setIconSize(QSize(16,16));
    actionsLayout->addWidget(bookFromScheduleButton);
    actionsLayout->addWidget(deleteSlotButton);
    actionsLayout->addWidget(addSlotInScheduleButton);
    actionsLayout->addStretch();
    actionsLayout->addWidget(backButton);
    layout->addLayout(actionsLayout);
    
    schedulePageIndex = stackedWidget->addWidget(schedulePage);

    connect(prevWeekButton, &QPushButton::clicked, this, &DoctorWidget::onPrevWeek);
    connect(nextWeekButton, &QPushButton::clicked, this, &DoctorWidget::onNextWeek);
    connect(todayButton, &QPushButton::clicked, this, &DoctorWidget::onToday);
    
    // Connect time slot duration spin box to reload schedule with new duration
    connect(timeSlotDurationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        selectedIntervalMinutes = value;
        loadSchedule();
    });
    
    connect(scheduleTable, &QTableWidget::cellClicked, this, &DoctorWidget::onCellClicked);
    connect(deleteSlotButton, &QPushButton::clicked, this, [this]() {
        QTableWidgetItem *item = scheduleTable->currentItem();
        if (!item) {
            QMessageBox::warning(this, "Ошибка", "Пожалуйста, выберите слот для удаления");
            return;
        }
        int schId = item->data(Qt::UserRole).toInt();
        if (schId <= 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите существующий слот");
            return;
        }

        AppointmentSchedule sch = dataManager.getScheduleById(schId);
        QString st = sch.status.trimmed().toLower();
        if (st == "booked") {
            QMessageBox::warning(this, "Ошибка", "Нельзя удалить занятой слот. Завершите приём или снимите бронь.");
            return;
        }

        QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтвердите удаление",
            "Вы уверены, что хотите удалить выбранный слот?", QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) return;

        dataManager.deleteSchedule(schId);
        QMessageBox::information(this, "Готово", "Слот удалён");
        loadSchedule();
    });
    connect(bookFromScheduleButton, &QPushButton::clicked, this, [this]() {
        QTableWidgetItem *item = scheduleTable->currentItem();
        if (!item) {
            QMessageBox::warning(this, "Ошибка", "Пожалуйста, выберите слот");
            return;
        }
        int schId = item->data(Qt::UserRole).toInt();
        if (schId <= 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите свободный слот");
            return;
        }
        DoctorVisitDialog dlg(currentUser.id, schId, 1, this);
        connect(&dlg, &DoctorVisitDialog::appointmentSaved, this, &DoctorWidget::onVisitCompleted);
        dlg.exec();
    });
    connect(addSlotInScheduleButton, &QPushButton::clicked, this, [this]() {
        AddSlotDialog dlg(currentUser.id, QTime(7,0), 20, this);
        connect(&dlg, &AddSlotDialog::slotAdded, this, [this]() {
            loadSchedule();
        });
        dlg.exec();
    });
    connect(backButton, &QPushButton::clicked, this, &DoctorWidget::onBackFromSchedule);
}

void DoctorWidget::setUser(const LoginUser &user) {
    currentUser = user;
    refreshHeader();
    stackedWidget->setCurrentIndex(mainPageIndex);
}

void DoctorWidget::refreshHeader() {
    Doctor d = dataManager.getDoctorById(currentUser.id);
    mainTitleLabel->setText(QString("Здравствуйте, %1!").arg(d.fname));
}

void DoctorWidget::onViewSchedule() {
    scheduleStartDate = QDate::currentDate();
    loadSchedule();
    stackedWidget->setCurrentIndex(schedulePageIndex);
}

void DoctorWidget::onPrevWeek() {
    scheduleStartDate = scheduleStartDate.addDays(-7);
    loadSchedule();
}

void DoctorWidget::onNextWeek() {
    scheduleStartDate = scheduleStartDate.addDays(7);
    loadSchedule();
}

void DoctorWidget::onToday() {
    scheduleStartDate = QDate::currentDate();
    loadSchedule();
}

void DoctorWidget::onAddSlot() {
    // Открыть диалог добавления слота
    AddSlotDialog dlg(currentUser.id, this);
    connect(&dlg, &AddSlotDialog::slotAdded, this, [this]() {
        loadSchedule();
    });
    dlg.exec();
    // После добавления слота вернуться на главную
    stackedWidget->setCurrentIndex(mainPageIndex);
}

void DoctorWidget::onBookAppointment() {
    // Открыть диалог записи пациента на прием
    DoctorVisitDialog dlg(currentUser.id, -1, 1, this);
    connect(&dlg, &DoctorVisitDialog::appointmentSaved, this, &DoctorWidget::onVisitCompleted);
    dlg.exec();
}

void DoctorWidget::loadSchedule() {
    scheduleTable->clear();
    scheduleTable->setRowCount(0);
    scheduleTable->setColumnCount(1);

    // Setup headers: time column + 7 days
    QStringList headers;
    headers << "Время";
    QDate start = scheduleStartDate;
    
    // Calculate start date to be Monday
    int dayOfWeek = start.dayOfWeek(); // 1 = Monday, 7 = Sunday (Qt default)
    if (dayOfWeek != 1) {
        start = start.addDays(1 - dayOfWeek); // Move to Monday of the same week
    }
    
    // Update week label showing range
    if (weekLabel) {
        QString label = QString("%1 — %2").arg(start.toString("dd.MM.yyyy"), start.addDays(6).toString("dd.MM.yyyy"));
        weekLabel->setText(label);
    }
    QDate today = QDate::currentDate();
    
    for (int i = 0; i < 7; ++i) {
        QDate dt = start.addDays(i);
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
    scheduleTable->setColumnCount(headers.size());
    scheduleTable->setHorizontalHeaderLabels(headers);

    // Collect all schedules for the doctor
    QList<AppointmentSchedule> schedules = dataManager.getDoctorSchedules(currentUser.id);

    // Use the interval selected by the user in the combo box
    int minimalInterval = selectedIntervalMinutes;

    // Build rows from 6:00 to 22:00 with step = minimalInterval minutes
    const int dayStartMin = 6 * 60;
    const int dayEndMin = 22 * 60; // exclusive end
    int totalMinutes = dayEndMin - dayStartMin;
    int rowsCount = (totalMinutes + minimalInterval - 1) / minimalInterval;
    scheduleTable->setRowCount(rowsCount);

    for (int r = 0; r < rowsCount; ++r) {
        int minutes = dayStartMin + r * minimalInterval;
        int hour = minutes / 60;
        int minute = minutes % 60;
        QTableWidgetItem *timeItem = new QTableWidgetItem(QString("%1:%2").arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0')));
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsSelectable);
        scheduleTable->setItem(r, 0, timeItem);
    }

    // Populate cells with schedule blocks using minute granularity
    for (const AppointmentSchedule &s : schedules) {
        int dayOffset = scheduleStartDate.daysTo(s.time_from.date());
        int column = 1 + dayOffset;
        if (column < 1 || column >= scheduleTable->columnCount()) continue;

        int startMinOfDay = s.time_from.time().hour() * 60 + s.time_from.time().minute();
        int endMinOfDay = s.time_to.time().hour() * 60 + s.time_to.time().minute();

        if (startMinOfDay < dayStartMin || endMinOfDay > dayEndMin) continue;

        int startIndex = (startMinOfDay - dayStartMin) / minimalInterval;
        int durationMin = endMinOfDay - startMinOfDay;
        int rowSpan = (durationMin + minimalInterval - 1) / minimalInterval;
        if (rowSpan <= 0) rowSpan = 1;

        // Determine status and color based on schedule status (case-insensitive)
        QString st = s.status.trimmed().toLower();
        QString statusText = "Свободен";
        QColor bgColor = QColor(144, 190, 109);
        if (st == "booked" || st == "busy") {
            statusText = "Занято";
            bgColor = QColor(255, 165, 0);
        }

        // Remove any existing widgets in the spanned area
        for (int r = startIndex; r < startIndex + rowSpan && r < scheduleTable->rowCount(); ++r) {
            QWidget *w = scheduleTable->cellWidget(r, column);
            if (w) { delete w; }
            scheduleTable->setItem(r, column, nullptr);
        }

        // Create item for the slot and place at the start index
        QTableWidgetItem *cell = new QTableWidgetItem(statusText);
        cell->setBackground(bgColor);
        cell->setForeground(Qt::white);
        cell->setTextAlignment(Qt::AlignCenter);
        cell->setData(Qt::UserRole, s.id_ap_sch);
        cell->setData(Qt::UserRole + 1, rowSpan);
        scheduleTable->setItem(startIndex, column, cell);

        if (rowSpan > 1) {
            scheduleTable->setSpan(startIndex, column, rowSpan, 1);
        }
    }
    
    // Highlight today's column with a light background color
    int todayColumn = -1;
    for (int i = 0; i < 7; ++i) {
        QDate dt = start.addDays(i);
        if (dt == today) {
            todayColumn = 1 + i;
            break;
        }
    }
    
    if (todayColumn >= 0) {
        QColor todayBgColor = QColor(200, 220, 255); // Light blue
        for (int r = 0; r < scheduleTable->rowCount(); ++r) {
            QTableWidgetItem *item = scheduleTable->item(r, todayColumn);
            if (!item) {
                item = new QTableWidgetItem();
                scheduleTable->setItem(r, todayColumn, item);
            }
            // Only apply background if no other content (to avoid overwriting schedule blocks)
            if (item->data(Qt::UserRole).toInt() <= 0) {
                item->setBackground(todayBgColor);
                item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
            }
        }
    }
}

void DoctorWidget::onCellClicked(int row, int column) {
    if (column == 0) return; // time column
    // Try to get the item at the clicked coordinates. If null (because of span), walk upwards to find top cell.
    QTableWidgetItem *it = scheduleTable->item(row, column);
    int probeRow = row;
    while (!it && probeRow >= 0) {
        --probeRow;
        it = scheduleTable->item(probeRow, column);
    }
    if (!it) return;
    // Ensure the clicked (or spanned) cell becomes the current selection
    scheduleTable->setCurrentCell(probeRow, column);
    int schId = it->data(Qt::UserRole).toInt();
    if (schId <= 0) return;
    
    AppointmentSchedule sch = dataManager.getScheduleById(schId);
    
    // Если слот свободен, не реагируем на клик
    QString st = sch.status.trimmed().toLower();
    if (st == "free" || st == "available" || st.isEmpty()) {
        return;
    }
    
    // Если слот занят, открываем диалог приема
    if (st == "booked") {
        DoctorVisitDialog dlg(currentUser.id, schId, 0, this);
        connect(&dlg, &DoctorVisitDialog::visitCompleted, this, &DoctorWidget::onVisitCompleted);
        dlg.exec();
    }
}

void DoctorWidget::onBackFromSchedule() {
    stackedWidget->setCurrentIndex(mainPageIndex);
}

void DoctorWidget::onVisitCompleted() {
    loadSchedule(); // Refresh schedule after completing visit
    stackedWidget->setCurrentIndex(mainPageIndex);
}

void DoctorWidget::onProfileClicked() {
    emit requestPageChange(1);
}

void DoctorWidget::onSettingsClicked() {
    emit requestPageChange(1);
}
