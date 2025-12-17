#include "createpatientdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDate>
#include <QTimer>
#include <QStyle>
#include <QIcon>
#include <QPixmap>
#include <QSize>

CreatePatientDialog::CreatePatientDialog(QWidget *parent, const Patient *existing)
    : QDialog(parent),
      dataManager(QCoreApplication::applicationDirPath() + "/../data") {
    setMinimumWidth(450);
    // If existing patient provided - open in edit mode
    if (existing) {
        editMode = true;
        editingPatientId = existing->id_patient;
        setWindowTitle("Редактирование пациента");
    } else {
        editMode = false;
        setWindowTitle("Создание пациента");
    }

    buildUI();

    // If edit - prefill fields with existing data
    if (editMode && existing) {
        createdPatient = *existing;
        firstNameEdit->setText(existing->fname);
        lastNameEdit->setText(existing->lname);
        middleNameEdit->setText(existing->tname);
        birthDateEdit->setDate(QDate::fromString(existing->bdate, "yyyy-MM-dd"));
        phoneEdit->setText(existing->phone_number);
        emailEdit->setText(existing->email);
        snilsEdit->setText(existing->snils);
        omsEdit->setText(existing->oms);
        createButton->setText("Обновить");
    }
}

void CreatePatientDialog::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    QLabel *titleLabel = new QLabel("Добавление члена семьи");
    titleLabel->setProperty("class", "dialog-title");
    main->addWidget(titleLabel);

    firstNameEdit = new QLineEdit();
    lastNameEdit = new QLineEdit();
    middleNameEdit = new QLineEdit();
    birthDateEdit = new QDateEdit();
    birthDateEdit->setCalendarPopup(true);
    birthDateEdit->setDisplayFormat("dd.MM.yyyy");
    birthDateEdit->setDate(QDate::currentDate());
    phoneEdit = new QLineEdit();
    emailEdit = new QLineEdit();
    snilsEdit = new QLineEdit();
    omsEdit = new QLineEdit();

    form->addRow("Имя:", firstNameEdit);
    form->addRow("Фамилия:", lastNameEdit);
    form->addRow("Отчество:", middleNameEdit);
    form->addRow("Дата рождения:", birthDateEdit);
    form->addRow("Телефон:", phoneEdit);
    form->addRow("Email:", emailEdit);
    form->addRow("СНИЛС:", snilsEdit);
    form->addRow("Полис ОМС:", omsEdit);

    statusLabel = new QLabel("");
    createButton = new QPushButton("Добавить");
    cancelButton = new QPushButton("Отмена");

    QHBoxLayout *actions = new QHBoxLayout();
    actions->addWidget(createButton);
    actions->addWidget(cancelButton);

    main->addLayout(form);
    main->addWidget(statusLabel);
    main->addLayout(actions);

    connect(createButton, &QPushButton::clicked, this, &CreatePatientDialog::onCreatePatient);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void CreatePatientDialog::onCreatePatient() {
    // Валидация
    if (firstNameEdit->text().isEmpty() || lastNameEdit->text().isEmpty()) {
        statusLabel->setText("Заполните имя и фамилию");
        statusLabel->setProperty("status", "error");
        return;
    }

    // Собираем данные пациента
    if (!editMode) {
        createdPatient.id_patient = dataManager.getNextPatientId();
    } else {
        createdPatient.id_patient = editingPatientId;
    }
    createdPatient.fname = firstNameEdit->text();
    createdPatient.lname = lastNameEdit->text();
    createdPatient.tname = middleNameEdit->text();
    createdPatient.bdate = birthDateEdit->date().toString("yyyy-MM-dd");
    createdPatient.phone_number = phoneEdit->text();
    createdPatient.email = emailEdit->text();
    createdPatient.snils = snilsEdit->text();
    createdPatient.oms = omsEdit->text();
    createdPatient.password = ""; // Пароль не требуется для членов семьи

    // Сохраняем в БД (создание или обновление)
    if (!editMode) {
        dataManager.addPatient(createdPatient);
        statusLabel->setText("Пациент добавлен");
        statusLabel->setProperty("status", "success");
    } else {
        dataManager.updatePatient(createdPatient);
        statusLabel->setText("Данные пациента обновлены");
        statusLabel->setProperty("status", "success");
    }
    emit patientCreated(createdPatient);
    // Ensure style is updated from QSS
    style()->unpolish(statusLabel);
    style()->polish(statusLabel);
    QTimer::singleShot(800, this, &QDialog::accept);
}

Patient CreatePatientDialog::getCreatedPatient() const {
    return createdPatient;
}
