#include "managers/bulkoperationsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

BulkOperationsDialog::BulkOperationsDialog(QWidget* parent)
    : QDialog(parent), m_dataManager(QString()) {
    setWindowTitle("Массовые операции — создать окна приема");
    resize(600, 300);
    buildUI();
}

void BulkOperationsDialog::buildUI() {
    QVBoxLayout* main = new QVBoxLayout(this);
    QHBoxLayout* row1 = new QHBoxLayout();
    m_doctorCombo = new QComboBox();
    m_doctorCombo->addItem("-- Выберите врача --", -1);
    QList<Doctor> docs = m_dataManager.getAllDoctors();
    for (const Doctor &d : docs) m_doctorCombo->addItem(d.fullName(), d.id_doctor);
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    row1->addWidget(new QLabel("Врач:"));
    row1->addWidget(m_doctorCombo);
    row1->addWidget(new QLabel("Дата:"));
    row1->addWidget(m_dateEdit);
    main->addLayout(row1);

    QHBoxLayout* row2 = new QHBoxLayout();
    m_startTimeEdit = new QTimeEdit(QTime(9,0));
    m_endTimeEdit = new QTimeEdit(QTime(18,0));
    row2->addWidget(new QLabel("Рабочий период:"));
    row2->addWidget(m_startTimeEdit);
    row2->addWidget(new QLabel("—"));
    row2->addWidget(m_endTimeEdit);
    main->addLayout(row2);

    QHBoxLayout* row3 = new QHBoxLayout();
    m_lunchFromEdit = new QTimeEdit(QTime(13,0));
    m_lunchToEdit = new QTimeEdit(QTime(14,0));
    m_intervalSpin = new QSpinBox();
    m_intervalSpin->setRange(5, 240);
    m_intervalSpin->setValue(20);
    row3->addWidget(new QLabel("Обед:"));
    row3->addWidget(m_lunchFromEdit);
    row3->addWidget(new QLabel("—"));
    row3->addWidget(m_lunchToEdit);
    row3->addStretch();
    row3->addWidget(new QLabel("Интервал, мин:"));
    row3->addWidget(m_intervalSpin);
    main->addLayout(row3);

    QHBoxLayout* footer = new QHBoxLayout();
    m_applyBtn = new QPushButton("Создать слоты");
    m_cancelBtn = new QPushButton("Отмена");
    footer->addStretch();
    footer->addWidget(m_applyBtn);
    footer->addWidget(m_cancelBtn);
    main->addLayout(footer);

    connect(m_applyBtn, &QPushButton::clicked, this, &BulkOperationsDialog::onApply);
    connect(m_cancelBtn, &QPushButton::clicked, this, &BulkOperationsDialog::reject);
}

void BulkOperationsDialog::onApply() {
    int docId = m_doctorCombo->currentData().toInt();
    if (docId <= 0) { QMessageBox::warning(this, "Ошибка", "Выберите врача"); return; }
    QDate date = m_dateEdit->date();
    QTime start = m_startTimeEdit->time();
    QTime end = m_endTimeEdit->time();
    QTime lunchFrom = m_lunchFromEdit->time();
    QTime lunchTo = m_lunchToEdit->time();
    int interval = m_intervalSpin->value();

    if (start >= end) { QMessageBox::warning(this, "Ошибка", "Некорректный рабочий период"); return; }
    if (lunchFrom >= lunchTo) { QMessageBox::warning(this, "Ошибка", "Некорректное время обеда"); return; }

    // Create slots in the selected period skipping lunch by interval
    QTime t = start;
    int created = 0;
    while (t.addSecs(interval * 60) <= end) {
        QTime slotEnd = t.addSecs(interval * 60);
        // skip lunch range
        if (!(slotEnd <= lunchFrom || t >= lunchTo)) {
            t = slotEnd;
            continue;
        }

        AppointmentSchedule sch;
        sch.id_ap_sch = m_dataManager.getNextScheduleId();
        sch.id_doctor = docId;
        sch.time_from = QDateTime(date, t);
        sch.time_to = QDateTime(date, slotEnd);
        sch.status = "free";

        if (m_dataManager.canAddSchedule(sch)) {
            m_dataManager.addSchedule(sch);
            created++;
        }
        t = slotEnd;
    }

    QMessageBox::information(this, "Готово", QString("Создано %1 слотов").arg(created));
    accept();
}
