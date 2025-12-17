#include "managers/bulkoperationsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

BulkOperationsDialog::BulkOperationsDialog(QWidget* parent)
    : QDialog(parent), m_dataManager(QString()) {
    setWindowTitle("Массовые операции — создать окна приема");
    resize(600, 300);
    loadDoctors();
    buildUI();
}

void BulkOperationsDialog::loadDoctors() {
    QList<Doctor> docs = m_dataManager.getAllDoctors();
    QStringList doctorNames;
    for (const Doctor &d : docs) {
        doctorNames.append(d.fullName());
    }
    doctorNames.sort();
    m_doctorCompleter = new QCompleter(doctorNames, this);
    m_doctorCompleter->setCaseSensitivity(Qt::CaseInsensitive);
}

void BulkOperationsDialog::buildUI() {
    QVBoxLayout* main = new QVBoxLayout(this);
    main->setContentsMargins(12, 12, 12, 12);
    main->setSpacing(8);
    
    QHBoxLayout* row1 = new QHBoxLayout();
    row1->setSpacing(8);
    m_doctorEdit = new QLineEdit();
    m_doctorEdit->setPlaceholderText("Введите имя врача...");
    m_doctorEdit->setCompleter(m_doctorCompleter);
    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    row1->addWidget(new QLabel("Врач:"), 0);
    row1->addWidget(m_doctorEdit, 1);
    row1->addWidget(new QLabel("Дата:"), 0);
    row1->addWidget(m_dateEdit, 0);
    main->addLayout(row1);

    QHBoxLayout* row2 = new QHBoxLayout();
    row2->setSpacing(8);
    m_startTimeEdit = new QTimeEdit(QTime(9,0));
    m_endTimeEdit = new QTimeEdit(QTime(18,0));
    row2->addWidget(new QLabel("Рабочий период:"), 0);
    row2->addWidget(m_startTimeEdit, 0);
    row2->addWidget(new QLabel("—"), 0);
    row2->addWidget(m_endTimeEdit, 0);
    row2->addStretch();
    main->addLayout(row2);

    QHBoxLayout* row3 = new QHBoxLayout();
    row3->setSpacing(8);
    m_lunchCheckBox = new QCheckBox("Обед:");
    m_lunchCheckBox->setChecked(true); // по умолчанию включен
    m_lunchFromEdit = new QTimeEdit(QTime(13,0));
    m_lunchToEdit = new QTimeEdit(QTime(14,0));
    m_intervalSpin = new QSpinBox();
    m_intervalSpin->setRange(5, 240);
    m_intervalSpin->setValue(20);
    row3->addWidget(m_lunchCheckBox, 0);
    row3->addWidget(m_lunchFromEdit, 0);
    row3->addWidget(new QLabel("—"), 0);
    row3->addWidget(m_lunchToEdit, 0);
    row3->addStretch();
    row3->addWidget(new QLabel("Интервал, мин:"), 0);
    row3->addWidget(m_intervalSpin, 0);
    main->addLayout(row3);

    QHBoxLayout* footer = new QHBoxLayout();
    footer->setSpacing(8);
    m_applyBtn = new QPushButton("Создать слоты");
    m_cancelBtn = new QPushButton("Отмена");
    footer->addStretch();
    footer->addWidget(m_applyBtn);
    footer->addWidget(m_cancelBtn);
    main->addLayout(footer);
    
    main->addStretch();

    connect(m_applyBtn, &QPushButton::clicked, this, &BulkOperationsDialog::onApply);
    connect(m_cancelBtn, &QPushButton::clicked, this, &BulkOperationsDialog::reject);
}

void BulkOperationsDialog::onApply() {
    QString doctorName = m_doctorEdit->text().trimmed();
    if (doctorName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя врача");
        return;
    }
    
    // Find doctor by name
    int docId = -1;
    QList<Doctor> allDoctors = m_dataManager.getAllDoctors();
    for (const Doctor &d : allDoctors) {
        if (d.fullName().toLower() == doctorName.toLower()) {
            docId = d.id_doctor;
            break;
        }
    }
    
    if (docId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Врач не найден");
        return;
    }
    
    QDate date = m_dateEdit->date();
    QTime start = m_startTimeEdit->time();
    QTime end = m_endTimeEdit->time();
    bool hasLunch = m_lunchCheckBox->isChecked();
    QTime lunchFrom = m_lunchFromEdit->time();
    QTime lunchTo = m_lunchToEdit->time();
    int interval = m_intervalSpin->value();

    if (start >= end) { QMessageBox::warning(this, "Ошибка", "Некорректный рабочий период"); return; }
    if (hasLunch && lunchFrom >= lunchTo) { QMessageBox::warning(this, "Ошибка", "Некорректное время обеда"); return; }

    // Create slots in the selected period skipping lunch by interval
    QTime t = start;
    int created = 0;
    while (t.addSecs(interval * 60) <= end) {
        QTime slotEnd = t.addSecs(interval * 60);
        // skip lunch range (only if lunch is enabled)
        if (hasLunch && !(slotEnd <= lunchFrom || t >= lunchTo)) {
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
