#include "addbycodedialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QIcon>
#include <QSize>

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
    instructionLabel->setProperty("class", "muted-text");
    mainLayout->addWidget(instructionLabel);

    mainLayout->addSpacing(12);

    // Код приглашения
    QLabel *codeLabel = new QLabel("Код приглашения:");
    codeInput = new QLineEdit();
    codeInput->setMaxLength(6);
    codeInput->setPlaceholderText("Введите 6-символный код");
    codeInput->setInputMask("AAAAAA");
    codeInput->setAlignment(Qt::AlignCenter);
    codeInput->setProperty("class", "invitation-code-input");
    // Focus на поле ввода для быстрого ввода кода
    codeInput->setFocus();

    mainLayout->addWidget(codeLabel);
    mainLayout->addWidget(codeInput);

    mainLayout->addSpacing(12);

    // Статус сообщение
    statusLabel = new QLabel();
    statusLabel->setProperty("class", "status-label");
    statusLabel->hide();
    mainLayout->addWidget(statusLabel);

    mainLayout->addStretch();

    // Кнопки
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("Присоединиться");
    addButton->setText(QString::fromUtf8("✅ ") + addButton->text());
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

    statusLabel->hide();

    // Проверка пустого поля
    if (code.isEmpty()) {
        statusLabel->setText("Введите код приглашения");
        statusLabel->setProperty("class", "status-label error");
        statusLabel->show();
        return;
    }

    // Проверка длины кода
    if (code.length() != 6) {
        statusLabel->setText("Код должен содержать ровно 6 символов");
        statusLabel->setProperty("class", "status-label error");
        statusLabel->show();
        return;
    }

    // Получить код приглашения из БД
    InvitationCode ic = dataManager.getInvitationCodeByCode(code);
    
    if (ic.id == 0) {
        statusLabel->setText("Код приглашения не найден");
        statusLabel->setProperty("class", "status-label error");
        statusLabel->show();
        return;
    }

    if (ic.used) {
        statusLabel->setText("Этот код уже использован");
        statusLabel->setProperty("class", "status-label error");
        statusLabel->show();
        return;
    }

    // Проверить, что пациент не состоит уже в какой-то семье
    QList<PatientGroup> existingFamilies = dataManager.getPatientFamilyMembers(patientId);
    if (!existingFamilies.isEmpty()) {
        statusLabel->setText("Вы уже состоите в семье. Нельзя состоять в нескольких семьях.");
        statusLabel->setProperty("class", "status-label error");
        statusLabel->show();
        return;
    }

    // Проверить, что код генерирует глава семьи (главу определяем по family_head в PatientGroup)
    // Коды должны быть уникальны, проверим это
    QList<PatientGroup> familyMembers = dataManager.getPatientFamilyMembers(ic.id_parent);
    
    if (!familyMembers.isEmpty()) {
        // Проверяем, что глава семьи совпадает с id_parent кода
        if (familyMembers.first().family_head != ic.id_parent) {
            statusLabel->setText("Ошибка: этот код генерирован не главой семьи");
            statusLabel->setProperty("class", "status-label error");
            statusLabel->show();
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
    
    statusLabel->setText("✓ Вы успешно присоединились к семье!");
    statusLabel->setProperty("class", "status-label success");
    statusLabel->show();
    
    emit patientAdded(ic.id_parent, headPatient.fullName());
    
    accept();
}
