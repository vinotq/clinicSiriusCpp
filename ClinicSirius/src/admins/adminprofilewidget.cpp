#include "admins/adminprofilewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QLabel>

AdminProfileWidget::AdminProfileWidget(QWidget *parent)
    : QWidget(parent), m_dataManager(QString()) {
    buildUI();
}

void AdminProfileWidget::buildUI() {
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
    idLayout->addWidget(new QLabel("ID администратора:"), 0);
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

    // Username
    QHBoxLayout* usernameLayout = new QHBoxLayout();
    usernameLayout->addWidget(new QLabel("Имя пользователя:"), 0);
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setPlaceholderText("Введите имя пользователя...");
    usernameLayout->addWidget(m_usernameEdit, 1);
    main->addLayout(usernameLayout);

    main->addSpacing(8);

    // Email
    QHBoxLayout* emailLayout = new QHBoxLayout();
    emailLayout->addWidget(new QLabel("Email:"), 0);
    m_emailEdit = new QLineEdit();
    m_emailEdit->setPlaceholderText("example@clinic.ru");
    emailLayout->addWidget(m_emailEdit, 1);
    main->addLayout(emailLayout);

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

    connect(m_saveBtn, &QPushButton::clicked, this, &AdminProfileWidget::onSaveSettings);
    connect(m_logoutBtn, &QPushButton::clicked, this, [this]() {
        emit requestLogout();
    });
}

void AdminProfileWidget::setUser(const LoginUser &user) {
    m_user = user;
    loadAdminInfo();
}

void AdminProfileWidget::loadAdminInfo() {
    Admin admin = m_dataManager.getAdminById(m_user.id);
    
    m_userIdLabel->setText(QString::number(admin.id));
    m_usernameEdit->setText(admin.username);
    m_emailEdit->setText(admin.email);
    m_passwordEdit->clear();
}

void AdminProfileWidget::onSaveSettings() {
    if (m_usernameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя пользователя");
        return;
    }

    Admin admin = m_dataManager.getAdminById(m_user.id);
    if (admin.id <= 0) {
        QMessageBox::warning(this, "Ошибка", "Администратор не найден");
        return;
    }
    
    admin.username = m_usernameEdit->text().trimmed();
    admin.email = m_emailEdit->text().trimmed();
    
    if (!m_passwordEdit->text().isEmpty()) {
        // CHANGED: Use hashPassword function for password hashing
        admin.password = hashPassword(m_passwordEdit->text());
    }
    
    m_dataManager.updateAdmin(admin);
    QMessageBox::information(this, "Успешно", "Профиль обновлен");
    m_passwordEdit->clear();
}

