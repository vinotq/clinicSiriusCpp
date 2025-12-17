#include "registrationwindow.h"
#include "datamanager.h"
#include "models.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>
#include <QRegularExpression>
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QTimer>
#include <QStyle>

RegistrationWindow::RegistrationWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    applyStyles();
}

RegistrationWindow::~RegistrationWindow() {
}

void RegistrationWindow::setupUI() {
    // Основной лейаут
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // Лого/Заголовок (иконка + текст)
    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoLayout->setAlignment(Qt::AlignCenter);
    QLabel *logoIcon = new QLabel();
    QPixmap clinicPix(":/images/clinic.svg");
    if (!clinicPix.isNull()) logoIcon->setPixmap(clinicPix.scaledToHeight(32, Qt::SmoothTransformation));
    logoIcon->setContentsMargins(0,0,8,0);
    QLabel *logoText = new QLabel("Клиника «Сириус»");
    QFont logoFont;
    logoFont.setPointSize(24);
    logoFont.setBold(true);
    logoText->setFont(logoFont);
    logoText->setAlignment(Qt::AlignCenter);
    logoLayout->addWidget(logoIcon);
    logoLayout->addWidget(logoText);
    mainLayout->addLayout(logoLayout);

    // Приветствие
    QLabel *welcomeLabel = new QLabel("Создайте аккаунт");
    QFont welcomeFont;
    welcomeFont.setPointSize(18);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(welcomeLabel);

    QLabel *subtitleLabel = new QLabel("Зарегистрируйтесь для получения доступа");
    QFont subtitleFont;
    subtitleFont.setPointSize(12);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(20);

    // Email поле
    QLabel *emailLabel = new QLabel("Email");
    QFont fieldLabelFont;
    fieldLabelFont.setPointSize(10);
    emailLabel->setFont(fieldLabelFont);
    mainLayout->addWidget(emailLabel);

    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("Введите ваш email");
    emailInput->setMinimumHeight(40);
    mainLayout->addWidget(emailInput);

    // Username поле
    QLabel *usernameLabel = new QLabel("Имя пользователя");
    usernameLabel->setFont(fieldLabelFont);
    mainLayout->addWidget(usernameLabel);

    usernameInput = new QLineEdit();
    usernameInput->setPlaceholderText("Выберите имя пользователя");
    usernameInput->setMinimumHeight(40);
    mainLayout->addWidget(usernameInput);

    QLabel *usernameHintLabel = new QLabel("Минимум 3 символа, только буквы и цифры");
    usernameHintLabel->setProperty("class", "hint-label");
    mainLayout->addWidget(usernameHintLabel);

    // Пароль поле
    QLabel *passwordLabel = new QLabel("Пароль");
    passwordLabel->setFont(fieldLabelFont);
    mainLayout->addWidget(passwordLabel);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Создайте пароль");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumHeight(40);
    passwordLayout->addWidget(passwordInput);

    passwordToggleButton = new QPushButton();
    passwordToggleButton->setMaximumWidth(45);
    passwordToggleButton->setMinimumHeight(40);
        passwordToggleButton->setText("👁️");
    passwordToggleButton->setIconSize(QSize(20,20));
    connect(passwordToggleButton, &QPushButton::clicked, this, &RegistrationWindow::onPasswordToggle);
    passwordLayout->addWidget(passwordToggleButton);

    mainLayout->addLayout(passwordLayout);

    QLabel *passwordHintLabel = new QLabel("Минимум 8 символов, содержит буквы, цифры и специальные символы");
    passwordHintLabel->setProperty("class", "hint-label");
    mainLayout->addWidget(passwordHintLabel);

    // Подтверждение пароля
    QLabel *confirmPasswordLabel = new QLabel("Подтвердите пароль");
    confirmPasswordLabel->setFont(fieldLabelFont);
    mainLayout->addWidget(confirmPasswordLabel);

    QHBoxLayout *confirmPasswordLayout = new QHBoxLayout();
    confirmPasswordInput = new QLineEdit();
    confirmPasswordInput->setPlaceholderText("Повторите пароль");
    confirmPasswordInput->setEchoMode(QLineEdit::Password);
    confirmPasswordInput->setMinimumHeight(40);
    connect(confirmPasswordInput, &QLineEdit::textChanged, this, &RegistrationWindow::validatePasswords);
    confirmPasswordLayout->addWidget(confirmPasswordInput);

    confirmPasswordToggleButton = new QPushButton();
    confirmPasswordToggleButton->setMaximumWidth(45);
    confirmPasswordToggleButton->setMinimumHeight(40);
        confirmPasswordToggleButton->setText("👁️");
    confirmPasswordToggleButton->setIconSize(QSize(20,20));
    connect(confirmPasswordToggleButton, &QPushButton::clicked, this, &RegistrationWindow::onConfirmPasswordToggle);
    confirmPasswordLayout->addWidget(confirmPasswordToggleButton);

    mainLayout->addLayout(confirmPasswordLayout);

    // Индикатор совпадения паролей
    passwordMatchLabel = new QLabel();
    mainLayout->addWidget(passwordMatchLabel);

    mainLayout->addSpacing(10);

    // Кнопка регистрации
    registerButton = new QPushButton("Зарегистрироваться");
    registerButton->setMinimumHeight(45);
    registerButton->setFont(QFont("Arial", 12, QFont::Bold));
    connect(registerButton, &QPushButton::clicked, this, &RegistrationWindow::onRegisterClicked);
    mainLayout->addWidget(registerButton);

    // Разделитель
    QLabel *orLabel = new QLabel("или");
    orLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(orLabel);

    // Ссылка на вход
    QHBoxLayout *loginLayout = new QHBoxLayout();
    QLabel *hasAccountLabel = new QLabel("Уже есть аккаунт?");
    loginButton = new QPushButton("Войдите");
    loginButton->setFlat(true);
    loginButton->setProperty("class", "login-link");
    connect(loginButton, &QPushButton::clicked, this, &RegistrationWindow::onLoginClicked);
    loginLayout->addStretch();
    loginLayout->addWidget(hasAccountLabel);
    loginLayout->addWidget(loginButton);
    loginLayout->addStretch();
    mainLayout->addLayout(loginLayout);

    // Сообщение об ошибке
    errorLabel = new QLabel();
    errorLabel->setWordWrap(true);
    mainLayout->addWidget(errorLabel);

    mainLayout->addStretch();

    setMinimumSize(500, 900);
}

void RegistrationWindow::applyStyles() {
    registerButton->setObjectName("registerButton");
}

void RegistrationWindow::onRegisterClicked() {
    if (!validateInput()) {
        return;
    }

    QString email = emailInput->text().trimmed();
    QString username = usernameInput->text().trimmed();
    QString password = passwordInput->text();

    // Регистрация пациента через DataManager
    DataManager dm("../data");
    
    // Проверка, не существует ли уже пользователь с таким email
    if (dm.emailExists(email)) {
        showError("Email уже зарегистрирован");
        return;
    }
    
    Patient newPatient;
    newPatient.id_patient = dm.getNextPatientId();
    newPatient.email = email;
    newPatient.fname = username;
    newPatient.password = hashPassword(password);  // Хешируем пароль с солью перед сохранением

    dm.addPatient(newPatient);
    
    showSuccess("Успешная регистрация! Перенаправление на вход...");
    
    // Переход на страницу входа через 2 секунды
    QTimer::singleShot(2000, this, &RegistrationWindow::onLoginClicked);
}

void RegistrationWindow::onLoginClicked() {
    emit switchToLogin();
}

void RegistrationWindow::onPasswordToggle() {
    if (passwordInput->echoMode() == QLineEdit::Password) {
        passwordInput->setEchoMode(QLineEdit::Normal);
           passwordToggleButton->setText("🙈");
    } else {
        passwordInput->setEchoMode(QLineEdit::Password);
           passwordToggleButton->setText("👁️");
    }
}

void RegistrationWindow::onConfirmPasswordToggle() {
    if (confirmPasswordInput->echoMode() == QLineEdit::Password) {
        confirmPasswordInput->setEchoMode(QLineEdit::Normal);
           confirmPasswordToggleButton->setText("🙈");
    } else {
        confirmPasswordInput->setEchoMode(QLineEdit::Password);
           confirmPasswordToggleButton->setText("👁️");
    }
}

void RegistrationWindow::validatePasswords() {
    if (passwordInput->text() != confirmPasswordInput->text()) {
        passwordMatchLabel->setText("Пароли не совпадают");
        passwordMatchLabel->setProperty("class", "hint-label-error");
        passwordMatchLabel->style()->unpolish(passwordMatchLabel);
        passwordMatchLabel->style()->polish(passwordMatchLabel);
        registerButton->setEnabled(false);
    } else if (!passwordInput->text().isEmpty()) {
        passwordMatchLabel->setText("Пароли совпадают");
        passwordMatchLabel->setProperty("class", "hint-label-success");
        passwordMatchLabel->style()->unpolish(passwordMatchLabel);
        passwordMatchLabel->style()->polish(passwordMatchLabel);
        registerButton->setEnabled(true);
    } else {
        passwordMatchLabel->setText("");
        registerButton->setEnabled(true);
    }
}

bool RegistrationWindow::validateInput() {
    QString email = emailInput->text().trimmed();
    QString username = usernameInput->text().trimmed();
    QString password = passwordInput->text();
    QString confirmPassword = confirmPasswordInput->text();

    // Проверка email
    QRegularExpression emailRegex("^[^\\s@]+@[^\\s@]+\\.[^\\s@]+$");
    if (!emailRegex.match(email).hasMatch()) {
        showError("Пожалуйста, введите корректный email");
        return false;
    }

    // Проверка username
    if (username.length() < 3) {
        showError("Имя пользователя должно содержать минимум 3 символа");
        return false;
    }

    QRegularExpression usernameRegex("^[a-zA-Z0-9]+$");
    if (!usernameRegex.match(username).hasMatch()) {
        showError("Имя пользователя может содержать только буквы и цифры");
        return false;
    }

    // Проверка пароля
    if (password.length() < 8) {
        showError("Пароль должен содержать минимум 8 символов");
        return false;
    }

    QRegularExpression passwordRegex("^(?=.*[a-zA-Z])(?=.*[0-9])(?=.*[!@#$%^&*])");
    if (!passwordRegex.match(password).hasMatch()) {
        showError("Пароль должен содержать буквы, цифры и специальные символы");
        return false;
    }

    // Проверка совпадения паролей
    if (password != confirmPassword) {
        showError("Пароли не совпадают");
        return false;
    }

    errorLabel->setText("");
    return true;
}

void RegistrationWindow::showError(const QString& message) {
    errorLabel->setText(message);
    errorLabel->setProperty("class", "error-label");
    errorLabel->style()->unpolish(errorLabel);
    errorLabel->style()->polish(errorLabel);
}

void RegistrationWindow::showSuccess(const QString& message) {
    errorLabel->setText(message);
    errorLabel->setProperty("class", "success-label");
    errorLabel->style()->unpolish(errorLabel);
    errorLabel->style()->polish(errorLabel);
}