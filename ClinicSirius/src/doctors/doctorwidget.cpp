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
    viewScheduleButton->setMinimumHeight(60);
    layout->addWidget(viewScheduleButton);
    
    addSlotButton = new QPushButton("➕ Добавить окно для приема");
    addSlotButton->setMinimumHeight(60);
    layout->addWidget(addSlotButton);
    
    bookAppointmentButton = new QPushButton("📝 Записать пациента на прием");
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
    prevWeekButton = new QPushButton("←");
    prevWeekButton->setMaximumWidth(40);
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
    
    scheduleTable = new QTableWidget();
    scheduleTable->setColumnCount(8); // one for time + 7 days
    scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    scheduleTable->verticalHeader()->setVisible(false);
    scheduleTable->setEditTriggers(QTableWidget::NoEditTriggers);
    scheduleTable->setSelectionMode(QTableWidget::SingleSelection);
    layout->addWidget(scheduleTable);
    
    // Кнопки действий
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    bookFromScheduleButton = new QPushButton("📝 Записать на выбранный слот");
    deleteSlotButton = new QPushButton("🗑 Удалить слот");
    // Новая кнопка
    addSlotInScheduleButton = new QPushButton("➕ Добавить окно для приема");
    backButton = new QPushButton("◀ Назад");
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
    // Update week label showing range
    if (weekLabel) {
        QString label = QString("%1 — %2").arg(start.toString("dd.MM.yyyy"), start.addDays(6).toString("dd.MM.yyyy"));
        weekLabel->setText(label);
    }
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

    // Collect all schedules for the doctor to determine minimal interval
    QList<AppointmentSchedule> schedules = dataManager.getDoctorSchedules(currentUser.id);

    // Determine grid step (in seconds) using GCD of all schedule durations and their offsets from day start.
    auto gcd = [](qint64 a, qint64 b) {
        if (a < 0) a = -a; if (b < 0) b = -b;
        while (b != 0) { qint64 t = a % b; a = b; b = t; }
        return a;
    };

    qint64 gcdSeconds = 0;
    const qint64 defaultIntervalSec = 20 * 60; // 20 minutes
    const qint64 dayStartSec = qint64(6 * 60 * 60);

    for (const AppointmentSchedule &s : schedules) {
        qint64 durSec = s.time_from.secsTo(s.time_to);
        if (durSec > 0) {
            gcdSeconds = gcdSeconds == 0 ? durSec : gcd(gcdSeconds, durSec);
        }
        // offset from day start in seconds
        qint64 startSec = qint64(s.time_from.time().hour()) * 3600 + qint64(s.time_from.time().minute()) * 60 + qint64(s.time_from.time().second());
        qint64 offsetSec = startSec - dayStartSec;
        if (offsetSec > 0) {
            gcdSeconds = gcdSeconds == 0 ? offsetSec : gcd(gcdSeconds, offsetSec);
        }
    }

    if (gcdSeconds <= 0) gcdSeconds = defaultIntervalSec;

    // Convert to minutes grid step; if gcdSeconds < 60 fall back to 1 minute.
    int minimalInterval = int(gcdSeconds / 60);
    if (minimalInterval < 1) minimalInterval = 1;

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
    stackedWidget->setCurrentIndex(mainPageIndex);
}

void DoctorWidget::onProfileClicked() {
    emit requestPageChange(1);
}

void DoctorWidget::onSettingsClicked() {
    emit requestPageChange(1);
}
