#include "managers/managerprofilewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

ManagerProfileWidget::ManagerProfileWidget(QWidget *parent)
    : QWidget(parent), m_dataManager(QString()) {
    buildUI();
}

void ManagerProfileWidget::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(16, 16, 16, 16);
    main->setSpacing(12);

    // Header
    QLabel* title = new QLabel("Мой профиль");
    QFont titleFont; titleFont.setPointSize(14); titleFont.setBold(true);
    title->setFont(titleFont);
    main->addWidget(title);

    // User ID (read-only)
    QHBoxLayout* idLayout = new QHBoxLayout();
    idLayout->addWidget(new QLabel("ID менеджера:"), 0);
    m_userIdLabel = new QLabel("—");
    QFont monoFont; monoFont.setFamily("Courier");
    m_userIdLabel->setFont(monoFont);
    idLayout->addWidget(m_userIdLabel, 0);
    idLayout->addStretch();
    main->addLayout(idLayout);

    main->addSpacing(12);

    // Form fields
    QLabel* formTitle = new QLabel("Основная информация");
    QFont formFont; formFont.setPointSize(11); formFont.setBold(true);
    formTitle->setFont(formFont);
    main->addWidget(formTitle);

    // First name
    QHBoxLayout* firstLayout = new QHBoxLayout();
    firstLayout->addWidget(new QLabel("Имя:"), 0);
    m_firstNameEdit = new QLineEdit();
    m_firstNameEdit->setPlaceholderText("Введите имя...");
    firstLayout->addWidget(m_firstNameEdit, 1);
    main->addLayout(firstLayout);

    // Last name
    QHBoxLayout* lastLayout = new QHBoxLayout();
    lastLayout->addWidget(new QLabel("Фамилия:"), 0);
    m_lastNameEdit = new QLineEdit();
    m_lastNameEdit->setPlaceholderText("Введите фамилию...");
    lastLayout->addWidget(m_lastNameEdit, 1);
    main->addLayout(lastLayout);

    // Patronymic (not used in Manager model, but kept for future extension)
    QHBoxLayout* patronymicLayout = new QHBoxLayout();
    patronymicLayout->addWidget(new QLabel("Отчество:"), 0);
    m_patronymicEdit = new QLineEdit();
    m_patronymicEdit->setPlaceholderText("(не используется)");
    m_patronymicEdit->setReadOnly(true);
    patronymicLayout->addWidget(m_patronymicEdit, 1);
    main->addLayout(patronymicLayout);

    main->addSpacing(8);

    // Email
    QHBoxLayout* emailLayout = new QHBoxLayout();
    emailLayout->addWidget(new QLabel("Email:"), 0);
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("example@clinic.ru");
    emailLayout->addWidget(m_emailEdit, 1);
    main->addLayout(emailLayout);

    // Phone (not used in Manager model, but kept for future extension)
    QHBoxLayout* phoneLayout = new QHBoxLayout();
    phoneLayout->addWidget(new QLabel("Телефон:"), 0);
    m_phoneEdit = new QLineEdit();
    m_phoneEdit->setPlaceholderText("(не используется)");
    m_phoneEdit->setReadOnly(true);
    phoneLayout->addWidget(m_phoneEdit, 1);
    main->addLayout(phoneLayout);

    main->addSpacing(12);

    // Password
    QLabel* securityTitle = new QLabel("Безопасность");
    securityTitle->setFont(formFont);
    main->addWidget(securityTitle);

    QHBoxLayout* passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(new QLabel("Новый пароль:"), 0);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setPlaceholderText("Оставьте пустым, если не хотите менять");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    passwordLayout->addWidget(m_passwordEdit, 1);
    main->addLayout(passwordLayout);

    main->addSpacing(16);
    main->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_saveBtn = new QPushButton("💾 Сохранить");
    m_logoutBtn = new QPushButton("🚪 Выход");
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_saveBtn);
    buttonLayout->addWidget(m_logoutBtn);
    main->addLayout(buttonLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &ManagerProfileWidget::onSaveSettings);
    connect(m_logoutBtn, &QPushButton::clicked, this, [this]() {
        emit requestLogout();
    });
}

void ManagerProfileWidget::setUser(const LoginUser &user) {
    m_user = user;
    loadManagerInfo();
}

void ManagerProfileWidget::loadManagerInfo() {
    Manager manager = m_dataManager.getManagerById(m_user.id);
    
    m_userIdLabel->setText(QString::number(manager.id));
    m_firstNameEdit->setText(manager.fname);
    m_lastNameEdit->setText(manager.lname);
    m_patronymicEdit->clear();  // Manager doesn't have patronymic field
    m_emailEdit->setText(manager.email);
    m_phoneEdit->clear();  // Manager doesn't have phone field
    m_passwordEdit->clear();
}

void ManagerProfileWidget::onSaveSettings() {
    if (m_firstNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя");
        return;
    }
    
    if (m_lastNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите фамилию");
        return;
    }

    Manager manager = m_dataManager.getManagerById(m_user.id);
    manager.fname = m_firstNameEdit->text();
    manager.lname = m_lastNameEdit->text();
    manager.email = m_emailEdit->text();
    // Note: patronymic and phone fields are not stored in Manager model
    
    if (!m_passwordEdit->text().isEmpty()) {
        // If password is provided, update it (should be hashed in real application)
        manager.password = m_passwordEdit->text();
    }
    
    m_dataManager.updateManager(manager);
    QMessageBox::information(this, "Успешно", "Профиль обновлен");
    m_passwordEdit->clear();
}
