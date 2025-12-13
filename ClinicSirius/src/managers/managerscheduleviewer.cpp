#include "managers/managerscheduleviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QMessageBox>
#include "patients/appointmentbookingwidget.h"

ManagerScheduleViewer::ManagerScheduleViewer(QWidget *parent)
    : QDialog(parent), m_dataManager(QString()) {
    m_startDate = QDate::currentDate();
    buildUI();
    loadDoctors();
}

void ManagerScheduleViewer::buildUI() {
    setWindowTitle("Расписание клиники");
    resize(900, 600);

    QVBoxLayout *main = new QVBoxLayout(this);

    QHBoxLayout *top = new QHBoxLayout();
    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("Поиск врача...");
    m_doctorCombo = new QComboBox();
    top->addWidget(m_filterEdit, 1);
    top->addWidget(m_doctorCombo);
    main->addLayout(top);

    QHBoxLayout *nav = new QHBoxLayout();
    m_prevBtn = new QPushButton("←");
    m_weekLabel = new QLabel();
    m_weekLabel->setAlignment(Qt::AlignCenter);
    m_nextBtn = new QPushButton("→");
    m_todayBtn = new QPushButton("Сегодня");
    nav->addWidget(m_prevBtn);
    nav->addStretch();
    nav->addWidget(m_weekLabel, 0);
    nav->addStretch();
    nav->addWidget(m_todayBtn);
    nav->addWidget(m_nextBtn);
    main->addLayout(nav);

    m_scheduleTable = new QTableWidget();
    m_scheduleTable->setColumnCount(8);
    m_scheduleTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_scheduleTable->verticalHeader()->setVisible(false);
    main->addWidget(m_scheduleTable, 1);

    connect(m_filterEdit, &QLineEdit::textChanged, this, &ManagerScheduleViewer::onDoctorFilterChanged);
    connect(m_doctorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ManagerScheduleViewer::onDoctorSelected);
    connect(m_prevBtn, &QPushButton::clicked, this, &ManagerScheduleViewer::onPrevWeek);
    connect(m_nextBtn, &QPushButton::clicked, this, &ManagerScheduleViewer::onNextWeek);
    connect(m_todayBtn, &QPushButton::clicked, this, &ManagerScheduleViewer::onToday);
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

        // Open booking widget with preselection
        QDialog dlg(this);
        dlg.setWindowTitle("Запись на слот");
        QVBoxLayout *l = new QVBoxLayout(&dlg);
        AppointmentBookingWidget *booking = new AppointmentBookingWidget(&dlg);
        LoginUser managerUser(LoginUser::MANAGER, -1, "Менеджер");
        booking->setUser(managerUser);
        booking->setInitialSelection(sch.id_doctor, sch.id_ap_sch);
        l->addWidget(booking);
        dlg.resize(800, 600);
        dlg.exec();

        // Reload schedules in case it was booked
        loadScheduleForDoctor(m_currentDoctorId);
    });
}

void ManagerScheduleViewer::loadDoctors() {
    m_allDoctors = m_dataManager.getAllDoctors();
    m_doctorCombo->clear();
    m_doctorCombo->addItem("-- Все врачи --", -1);
    for (const Doctor &d : m_allDoctors) {
        m_doctorCombo->addItem(d.fullName(), d.id_doctor);
    }
    m_currentDoctorId = -1;
    loadScheduleForDoctor(m_currentDoctorId);
}

void ManagerScheduleViewer::onDoctorFilterChanged(const QString &text) {
    m_doctorCombo->clear();
    m_doctorCombo->addItem("-- Все врачи --", -1);
    for (const Doctor &d : m_allDoctors) {
        if (text.isEmpty() || d.fullName().toLower().contains(text.toLower())) {
            m_doctorCombo->addItem(d.fullName(), d.id_doctor);
        }
    }
}

void ManagerScheduleViewer::onDoctorSelected(int index) {
    if (index < 0) return;
    m_currentDoctorId = m_doctorCombo->itemData(index).toInt();
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
    m_startDate = QDate::currentDate();
    loadScheduleForDoctor(m_currentDoctorId);
}

void ManagerScheduleViewer::loadScheduleForDoctor(int doctorId) {
    m_scheduleTable->clear();
    m_scheduleTable->setRowCount(0);
    m_scheduleTable->setColumnCount(1);

    QStringList headers;
    headers << "Время";
    QDate start = m_startDate;
    if (m_weekLabel) {
        QString label = QString("%1 — %2").arg(start.toString("dd.MM.yyyy"), start.addDays(6).toString("dd.MM.yyyy"));
        m_weekLabel->setText(label);
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
    m_scheduleTable->setColumnCount(headers.size());
    m_scheduleTable->setHorizontalHeaderLabels(headers);

    // For aggregate view: collect schedules
    QList<AppointmentSchedule> schedules;
    if (doctorId <= 0) {
        // All doctors: gather all schedules
        schedules = m_dataManager.getAllSchedules();
    } else {
        schedules = m_dataManager.getDoctorSchedules(doctorId);
    }

    // Use same logic as DoctorWidget for grid
    qint64 gcdSeconds = 0;
    const qint64 defaultIntervalSec = 20 * 60; // 20 minutes
    const qint64 dayStartSec = qint64(6 * 60 * 60);

    auto gcd = [](qint64 a, qint64 b) {
        if (a < 0) a = -a; if (b < 0) b = -b;
        while (b != 0) { qint64 t = a % b; a = b; b = t; }
        return a;
    };

    for (const AppointmentSchedule &s : schedules) {
        qint64 durSec = s.time_from.secsTo(s.time_to);
        if (durSec > 0) gcdSeconds = gcdSeconds == 0 ? durSec : gcd(gcdSeconds, durSec);
        qint64 startSec = qint64(s.time_from.time().hour()) * 3600 + qint64(s.time_from.time().minute()) * 60;
        qint64 offsetSec = startSec - dayStartSec;
        if (offsetSec > 0) gcdSeconds = gcdSeconds == 0 ? offsetSec : gcd(gcdSeconds, offsetSec);
    }
    if (gcdSeconds <= 0) gcdSeconds = defaultIntervalSec;
    int minimalInterval = int(gcdSeconds / 60);
    if (minimalInterval < 1) minimalInterval = 1;

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
        timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsSelectable);
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
        if (st == "booked" || st == "busy") {
            statusText = "Занято";
            bgColor = QColor(255, 165, 0);
        }

        QTableWidgetItem *cell = new QTableWidgetItem(statusText);
        cell->setBackground(bgColor);
        cell->setForeground(Qt::white);
        cell->setTextAlignment(Qt::AlignCenter);
        cell->setData(Qt::UserRole, s.id_ap_sch);
        m_scheduleTable->setItem(startIndex, column, cell);
        if (rowSpan > 1) {
            m_scheduleTable->setSpan(startIndex, column, rowSpan, 1);
        }
    }
}
