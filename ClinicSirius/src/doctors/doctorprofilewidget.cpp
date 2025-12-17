#include "doctorprofilewidget.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDate>
#include <QTimer>
#include <QIcon>

DoctorProfileWidget::DoctorProfileWidget(QWidget *parent)
    : QWidget(parent),
      dataManager(QCoreApplication::applicationDirPath() + "/../data") {
    buildUI();
}

void DoctorProfileWidget::buildUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    tabs = new QTabWidget(this);
    mainLayout->addWidget(tabs);

    // ------- Профиль -------
    QWidget *profileTab = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profileTab);

    QWidget *infoSection = new QWidget();
    infoSection->setProperty("class", "profile-section");
    QFormLayout *infoForm = new QFormLayout(infoSection);
    infoForm->setLabelAlignment(Qt::AlignLeft);
    nameValue = new QLabel("-");
    emailValue = new QLabel("-");
    phoneValue = new QLabel("-");
    birthValue = new QLabel("-");
    specValue = new QLabel("-");
    infoForm->addRow("Имя:", nameValue);
    infoForm->addRow("Email:", emailValue);
    infoForm->addRow("Телефон:", phoneValue);
    infoForm->addRow("Дата рождения:", birthValue);
    infoForm->addRow("Специальность:", specValue);
    profileLayout->addWidget(infoSection);
    profileLayout->addStretch();
    tabs->addTab(profileTab, "Профиль");

    // ------- Настройки -------
    QWidget *settingsTab = new QWidget();
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsTab);

    QWidget *settingsSection = new QWidget();
    settingsSection->setProperty("class", "profile-section");
    QFormLayout *settingsForm = new QFormLayout(settingsSection);
    firstNameEdit = new QLineEdit();
    lastNameEdit = new QLineEdit();
    middleNameEdit = new QLineEdit();
    birthEdit = new QDateEdit();
    birthEdit->setCalendarPopup(true);
    birthEdit->setDisplayFormat("yyyy-MM-dd");
    emailEdit = new QLineEdit();
    phoneEdit = new QLineEdit();
    
    settingsForm->addRow("Имя", firstNameEdit);
    settingsForm->addRow("Фамилия", lastNameEdit);
    settingsForm->addRow("Отчество", middleNameEdit);
    settingsForm->addRow("Дата рождения", birthEdit);
    settingsForm->addRow("Email", emailEdit);
    settingsForm->addRow("Телефон", phoneEdit);
    
    settingsLayout->addWidget(settingsSection);

    saveStatusLabel = new QLabel();
    saveButton = new QPushButton("Сохранить");
    saveButton->setIcon(QIcon(":/images/icon-save.svg"));
    saveButton->setIconSize(QSize(16,16));
    deleteButton = new QPushButton("Удалить профиль");
    deleteButton->setIcon(QIcon(":/images/icon-trash.svg"));
    deleteButton->setIconSize(QSize(16,16));

    QHBoxLayout *actions = new QHBoxLayout();
    actions->addWidget(saveButton);
    actions->addStretch();
    actions->addWidget(deleteButton);
    settingsLayout->addLayout(actions);
    settingsLayout->addWidget(saveStatusLabel);
    settingsLayout->addStretch();
    
    tabs->addTab(settingsTab, "Настройки");

    connect(saveButton, &QPushButton::clicked, this, &DoctorProfileWidget::onSaveProfile);
    connect(deleteButton, &QPushButton::clicked, this, &DoctorProfileWidget::onDeleteAccount);
}

void DoctorProfileWidget::setUser(const LoginUser &user) {
    currentUser = user;
    loadProfile();
}

void DoctorProfileWidget::loadProfile() {
    Doctor doctor = dataManager.getDoctorById(currentUser.id);
    
    // Загрузить профиль
    nameValue->setText(doctor.fullName());
    emailValue->setText(doctor.email);
    phoneValue->setText(doctor.phone_number);
    birthValue->setText(doctor.bdate.toString("dd.MM.yyyy"));
    
    // Загрузить специальность
    Specialization spec = dataManager.getSpecializationById(doctor.id_spec);
    specValue->setText(spec.name);

    // Заполнить форму редактирования
    firstNameEdit->setText(doctor.fname);
    lastNameEdit->setText(doctor.lname);
    middleNameEdit->setText(doctor.tname);
    birthEdit->setDate(doctor.bdate);
    emailEdit->setText(doctor.email);
    phoneEdit->setText(doctor.phone_number);
}

void DoctorProfileWidget::onSaveProfile() {
    // Validate required fields
    if (firstNameEdit->text().isEmpty() || lastNameEdit->text().isEmpty()) {
        saveStatusLabel->setText("Имя и фамилия обязательны.");
        return;
    }
    
    // Validate email format (simple check)
    QString email = emailEdit->text();
    if (!email.isEmpty() && !email.contains("@")) {
        saveStatusLabel->setText("Неверный формат email.");
        return;
    }
    
    // Validate phone (only digits if not empty)
    QString phone = phoneEdit->text();
    if (!phone.isEmpty()) {
        bool allDigits = true;
        for (QChar c : phone) {
            if (!c.isDigit()) {
                allDigits = false;
                break;
            }
        }
        if (!allDigits) {
            saveStatusLabel->setText("Телефон должен содержать только цифры.");
            return;
        }
    }
    
    Doctor doctor = dataManager.getDoctorById(currentUser.id);
    doctor.fname = firstNameEdit->text();
    doctor.lname = lastNameEdit->text();
    doctor.tname = middleNameEdit->text();
    doctor.bdate = birthEdit->date();
    doctor.email = emailEdit->text();
    doctor.phone_number = phoneEdit->text();

    dataManager.updateDoctor(doctor);
    saveStatusLabel->setText("Профиль сохранен");
    QTimer::singleShot(2000, [this]() {
        saveStatusLabel->setText("");
    });
    loadProfile();
}

void DoctorProfileWidget::onDeleteAccount() {
    // Check for associated schedules and appointments
    QList<AppointmentSchedule> schedules = dataManager.getDoctorSchedules(currentUser.id);
    QList<Appointment> appointments = dataManager.getAppointmentsByDoctor(currentUser.id);
    
    QString deleteMsg = "Вы уверены, что хотите удалить свой профиль? Это действие необратимо.";
    if (!schedules.isEmpty() || !appointments.isEmpty()) {
        deleteMsg += QString("\n\nВнимание: У вас есть ");
        if (!schedules.isEmpty()) {
            deleteMsg += QString("%1 активных слотов расписания").arg(schedules.count());
        }
        if (!appointments.isEmpty()) {
            if (!schedules.isEmpty()) deleteMsg += " и ";
            deleteMsg += QString("%1 записанных приемов").arg(appointments.count());
        }
        deleteMsg += ". Все они будут удалены.";
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, 
        "Удаление профиля", 
        deleteMsg,
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Delete associated schedules
        for (const auto& sch : schedules) {
            dataManager.deleteSchedule(sch.id_ap_sch);
        }
        
        // Delete associated appointments
        for (const auto& app : appointments) {
            dataManager.deleteAppointment(app.id_ap);
        }
        
        // Delete the doctor account
        dataManager.deleteDoctor(currentUser.id);
        emit requestAccountDeletion();
    }
}

void DoctorProfileWidget::openSettingsTab() {
    if (tabs) {
        tabs->setCurrentIndex(1);
    }
}
