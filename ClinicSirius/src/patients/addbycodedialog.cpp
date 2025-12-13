#include "addbycodedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

AddByCodeDialog::AddByCodeDialog(int patientId, QWidget *parent)
    : QDialog(parent), patientId(patientId), headPatientId(-1) {
    setWindowTitle("Присоединиться к семье");
    setMinimumWidth(400);
    setupUI();
}

void AddByCodeDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Инструкция
    QLabel *instructionLabel = new QLabel("Введите 6-символный код для присоединения к семье:");
    instructionLabel->setStyleSheet("color: #666; font-size: 11pt;");
    mainLayout->addWidget(instructionLabel);

    mainLayout->addSpacing(12);

    // Код приглашения
    QLabel *codeLabel = new QLabel("Код приглашения:");
    codeInput = new QLineEdit();
    codeInput->setMaxLength(6);
    codeInput->setPlaceholderText("Введите 6-символный код");
    codeInput->setAlignment(Qt::AlignCenter);
    codeInput->setStyleSheet("font-size: 14pt; letter-spacing: 2px; font-weight: bold;");

    mainLayout->addWidget(codeLabel);
    mainLayout->addWidget(codeInput);

    mainLayout->addSpacing(12);

    // Сообщение об ошибке
    errorLabel = new QLabel();
    errorLabel->setStyleSheet("color: #ef4444; font-weight: bold;");
    errorLabel->hide();
    mainLayout->addWidget(errorLabel);

    mainLayout->addStretch();

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("✓ Присоединиться");
    addButton->setMinimumHeight(36);
    QPushButton *cancelButton = new QPushButton("Отмена");
    cancelButton->setMinimumHeight(36);

    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Сигналы
    connect(addButton, &QPushButton::clicked, this, &AddByCodeDialog::onAddClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void AddByCodeDialog::onAddClicked() {
    QString code = codeInput->text().toUpper().trimmed();

    errorLabel->hide();

    // Проверка пустого поля
    if (code.isEmpty()) {
        errorLabel->setText("Введите код приглашения");
        errorLabel->show();
        return;
    }

    // Проверка длины кода
    if (code.length() != 6) {
        errorLabel->setText("Код должен содержать ровно 6 символов");
        errorLabel->show();
        return;
    }

    // Получить код приглашения из БД
    InvitationCode ic = dataManager.getInvitationCodeByCode(code);
    
    if (ic.id == 0) {
        errorLabel->setText("Код приглашения не найден");
        errorLabel->show();
        return;
    }

    if (ic.used) {
        errorLabel->setText("Этот код уже использован");
        errorLabel->show();
        return;
    }

    // Проверить, что пациент не состоит уже в какой-то семье
    QList<PatientGroup> existingFamilies = dataManager.getPatientFamilyMembers(patientId);
    if (!existingFamilies.isEmpty()) {
        errorLabel->setText("Вы уже состоите в семье. Нельзя состоять в нескольких семьях.");
        errorLabel->show();
        return;
    }

    // Проверить, что код генерирует глава семьи (главу определяем по family_head в PatientGroup)
    // Коды должны быть уникальны, проверим это
    QList<PatientGroup> familyMembers = dataManager.getPatientFamilyMembers(ic.id_parent);
    
    if (!familyMembers.isEmpty()) {
        // Проверяем, что глава семьи совпадает с id_parent кода
        if (familyMembers.first().family_head != ic.id_parent) {
            errorLabel->setText("Ошибка: этот код генерирован не главой семьи");
            errorLabel->show();
            return;
        }
    }

    // Все проверки пройдены - добавляем в семью
    PatientGroup newGroup;
    newGroup.id_patient_group = dataManager.getNextPatientGroupId();
    newGroup.id_parent = ic.id_parent;
    newGroup.id_child = patientId;
    newGroup.family_head = ic.id_parent;  // Глава семьи - тот, кто выпустил код
    
    dataManager.addFamilyMember(newGroup);

    // Отметить код как использованный
    dataManager.useInvitationCode(code, patientId);

    headPatientId = ic.id_parent;
    Patient headPatient = dataManager.getPatientById(ic.id_parent);
    emit patientAdded(ic.id_parent, headPatient.fullName());
    
    QMessageBox::information(this, "Успешно", "Вы успешно присоединились к семье!");
    accept();
}
