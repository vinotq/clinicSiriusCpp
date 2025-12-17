#include "addslotdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCoreApplication>
#include <QLabel>

AddSlotDialog::AddSlotDialog(int doctorId, QWidget *parent)
    : QDialog(parent), doctorId(doctorId),
      dataManager(QCoreApplication::applicationDirPath() + "/../data"),
      _defaultStartTime(QTime(10, 0)), _defaultDurationMin(20)
{
    setWindowTitle("Добавить окно для приема");
    setMinimumWidth(400);
    buildUI();
    loadRooms();
}

AddSlotDialog::AddSlotDialog(int doctorId_, QTime defaultStartTime, int defaultDurationMin, QWidget *parent)
    : QDialog(parent), doctorId(doctorId_),
      dataManager(QCoreApplication::applicationDirPath() + "/../data"),
      _defaultStartTime(defaultStartTime), _defaultDurationMin(defaultDurationMin)
{
    setWindowTitle("Добавить окно для приема");
    setMinimumWidth(400);
    buildUI();
    loadRooms();
}

void AddSlotDialog::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    QLabel *titleLabel = new QLabel("Добавление окна приема");
    titleLabel->setProperty("class", "dialog-title");
    main->addWidget(titleLabel);

    QLabel *infoLabel = new QLabel("Вы можете создать плавающее окно приема на любое время в диапазоне от 6:00 до 22:00");
    infoLabel->setWordWrap(true);
    infoLabel->setProperty("class", "muted-text");
    main->addWidget(infoLabel);

    dateEdit = new QDateEdit();
    dateEdit->setDate(QDate::currentDate().addDays(1)); // По умолчанию завтра
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("dd.MM.yyyy");

    startTimeEdit = new QTimeEdit();
    startTimeEdit->setTime(_defaultStartTime);
    startTimeEdit->setDisplayFormat("HH:mm");

    durationSpinBox = new QSpinBox();
    durationSpinBox->setMinimum(15);
    durationSpinBox->setMaximum(480);
    durationSpinBox->setSingleStep(15);
    durationSpinBox->setValue(_defaultDurationMin);
    durationSpinBox->setSuffix(" мин");

    // Create duration layout with preset buttons
    QHBoxLayout *durationLayout = new QHBoxLayout();
    durationLayout->addWidget(durationSpinBox);
    
    preset30 = new QPushButton("30 мин");
    preset30->setMaximumWidth(70);
    preset45 = new QPushButton("45 мин");
    preset45->setMaximumWidth(70);
    preset60 = new QPushButton("60 мин");
    preset60->setMaximumWidth(70);
    
    durationLayout->addWidget(preset30);
    durationLayout->addWidget(preset45);
    durationLayout->addWidget(preset60);
    durationLayout->addStretch();

    roomCombo = new QComboBox();

    form->addRow("Дата:", dateEdit);
    form->addRow("Время начала:", startTimeEdit);
    form->addRow("Длительность:", durationLayout);
    form->addRow("Кабинет:", roomCombo);

    addButton = new QPushButton("Добавить");
    cancelButton = new QPushButton("Отмена");

    QHBoxLayout *actions = new QHBoxLayout();
    actions->addWidget(addButton);
    actions->addWidget(cancelButton);

    main->addLayout(form);
    main->addLayout(actions);

    connect(addButton, &QPushButton::clicked, this, &AddSlotDialog::onAddSlot);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // Connect preset duration buttons
    connect(preset30, &QPushButton::clicked, [this]() { durationSpinBox->setValue(30); });
    connect(preset45, &QPushButton::clicked, [this]() { durationSpinBox->setValue(45); });
    connect(preset60, &QPushButton::clicked, [this]() { durationSpinBox->setValue(60); });
}

void AddSlotDialog::loadRooms() {
    roomCombo->clear();
    QList<Room> rooms = dataManager.getAllRooms();
    for (const Room &r : rooms) {
        roomCombo->addItem(QString("Кабинет %1").arg(r.room_number), r.id_room);
    }
    
    if (rooms.isEmpty()) {
        // Если нет кабинетов, добавить значение по умолчанию
        roomCombo->addItem("Кабинет 1", 1);
    }
}

void AddSlotDialog::onAddSlot() {
    // Validate duration
    if (durationSpinBox->value() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Длительность должна быть больше 0 минут.");
        return;
    }
    
    QTime startT = startTimeEdit->time();
    // Validate start time is within 6:00-22:00
    if (startT < QTime(6, 0) || startT >= QTime(22, 0)) {
        QMessageBox::warning(this, "Ошибка", "Время начала должно быть между 6:00 и 22:00.");
        return;
    }
    
    QDateTime from = QDateTime(dateEdit->date(), startTimeEdit->time());
    QDateTime to = from.addSecs(durationSpinBox->value() * 60);
    
    // Validate end time does not exceed 22:00
    if (to.time() > QTime(22, 0)) {
        QMessageBox::warning(this, "Ошибка", "Окончание не может быть после 22:00.");
        return;
    }

    AppointmentSchedule schedule;
    schedule.id_ap_sch = dataManager.getNextScheduleId();
    schedule.id_doctor = doctorId;
    schedule.id_room = roomCombo->currentData().toInt();
    schedule.time_from = from;
    schedule.time_to = to;

    // Валидация: проверяем пересечения с существующими окнами
    // 1) У врача — нельзя добавлять пересекающиеся окна
    QList<AppointmentSchedule> doctorSchedules = dataManager.getDoctorSchedules(doctorId);
    for (const AppointmentSchedule &s : doctorSchedules) {
        if (s.time_from.isValid() && s.time_to.isValid()) {
            // Пересечение интервалов: [from, to) пересекается с [s.time_from, s.time_to)
            if (from < s.time_to && s.time_from < to) {
                QMessageBox::warning(this, "Ошибка", "Новое окно пересекается с уже существующим окном врача.");
                return;
            }
        }
    }

    // 2) Тот же кабинет — нельзя добавлять пересекающиеся окна в одном кабинете
    QList<AppointmentSchedule> allSchedules = dataManager.getAllSchedules();
    for (const AppointmentSchedule &s : allSchedules) {
        if (s.id_room == schedule.id_room && s.time_from.isValid() && s.time_to.isValid()) {
            if (from < s.time_to && s.time_from < to) {
                QMessageBox::warning(this, "Ошибка", "Новое окно пересекается с уже существующим окном в выбранном кабинете.");
                return;
            }
        }
    }

    // Если проверка пройдена — добавить
    dataManager.addSchedule(schedule);

    QMessageBox::information(this, "Готово", "Окно приема добавлено");
    emit slotAdded();
    accept();
}
