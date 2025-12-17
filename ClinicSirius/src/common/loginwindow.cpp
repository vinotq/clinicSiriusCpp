#include "loginwindow.h"
#include "datamanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QMessageBox>
#include <QFont>
#include <QLabel>
#include <QDebug>
#include <QCoreApplication>
#include <QStyle>
#include <QPlainTextEdit>
#include <QClipboard>
#include <QGuiApplication>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    applyStyles();
}

LoginWindow::~LoginWindow() {
}

void LoginWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoLayout->setAlignment(Qt::AlignCenter);
    QLabel *logoIcon = new QLabel("🏥");
    logoIcon->setProperty("class", "header-logo-icon");
    QFont lic; lic.setPointSize(400); logoIcon->setFont(lic);
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

    QLabel *welcomeLabel = new QLabel("Добро пожаловать");
    QFont welcomeFont;
    welcomeFont.setPointSize(18);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(welcomeLabel);

    QLabel *subtitleLabel = new QLabel("Войдите в свой аккаунт");
    QFont subtitleFont;
    subtitleFont.setPointSize(12);
    subtitleLabel->setFont(subtitleFont);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setProperty("class", "login-subtitle");
    mainLayout->addWidget(subtitleLabel);

    mainLayout->addSpacing(20);

    QLabel *emailLabel = new QLabel("Email");
    emailLabel->setProperty("class", "field-label");
    mainLayout->addWidget(emailLabel);

    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("Введите email");
    emailInput->setMinimumHeight(40);
    mainLayout->addWidget(emailInput);

    QLabel *passwordLabel = new QLabel("Пароль");
    passwordLabel->setProperty("class", "field-label");
    mainLayout->addWidget(passwordLabel);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Введите пароль");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumHeight(40);
    passwordLayout->addWidget(passwordInput);

    passwordToggleButton = new QPushButton("👁️");
    passwordToggleButton->setMaximumWidth(45);
    passwordToggleButton->setMinimumHeight(40);
    passwordToggleButton->setProperty("class", "emoji-button");
    connect(passwordToggleButton, &QPushButton::clicked, this, &LoginWindow::onPasswordToggle);
    passwordLayout->addWidget(passwordToggleButton);

    mainLayout->addLayout(passwordLayout);

    mainLayout->addSpacing(10);

    QPushButton *forgotButton = new QPushButton("🔑 Забыли пароль?");
    forgotButton->setToolTip("Напомнить пароль");
    forgotButton->setFlat(true);
    forgotButton->setProperty("class", "login-forgot");
    mainLayout->addWidget(forgotButton, 0, Qt::AlignRight);

    mainLayout->addSpacing(10);

    loginButton = new QPushButton("🔐 Войти");
    loginButton->setMinimumHeight(45);
    loginButton->setFont(QFont("Arial", 12, QFont::Bold));
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    mainLayout->addWidget(loginButton);

    QLabel *orLabel = new QLabel("или");
    orLabel->setAlignment(Qt::AlignCenter);
    orLabel->setProperty("class", "login-separator");
    mainLayout->addWidget(orLabel);

    QHBoxLayout *registrationLayout = new QHBoxLayout();
    QLabel *noAccountLabel = new QLabel("Нет аккаунта?");
    registrationButton = new QPushButton("📝 Зарегистрироваться");
    registrationButton->setFlat(true);
    registrationButton->setProperty("class", "login-forgot");
    connect(registrationButton, &QPushButton::clicked, this, &LoginWindow::onRegistrationClicked);
    registrationLayout->addStretch();
    registrationLayout->addWidget(noAccountLabel);
    registrationLayout->addWidget(registrationButton);
    registrationLayout->addStretch();
    mainLayout->addLayout(registrationLayout);

    QWidget *testUsersPanel = new QWidget();
    testUsersPanel->setProperty("class", "test-users-panel");
    testUsersPanel->setAttribute(Qt::WA_StyledBackground, true);
    QVBoxLayout *testLayout = new QVBoxLayout(testUsersPanel);
    testLayout->setContentsMargins(20, 20, 20, 20);
    testLayout->setSpacing(8);

    QLabel *testTitle = new QLabel("ℹ️ Тестовые пользователи");
    QFont testTitleFont;
    testTitleFont.setPointSize(11);
    testTitleFont.setBold(true);
    testTitle->setFont(testTitleFont);
    testLayout->addWidget(testTitle);

    testUsersText = new QPlainTextEdit();
    testUsersText->setProperty("class", "test-users-text");
    testUsersText->setAttribute(Qt::WA_StyledBackground, true);
    testUsersText->setReadOnly(true);
    
    testUsersText->setPlainText(
        "тестовые пользователи:\n\n"
        "пациент  –  anna.ivanova@mail.ru\n"
        "доктор   –  igor.semenov@clinicsirius.ru\n"
        "менеджер –  sidorov@clinicsirius.ru\n"
        "админ    –  admin@clinicsirius.ru\n\n"
        "пароль ко всем пользователям – pass123"
    );
    testLayout->addWidget(testUsersText);

    QHBoxLayout *copyLayout = new QHBoxLayout();
    copyLayout->addStretch();
    copyUsersButton = new QPushButton("📋 Скопировать данные");
    copyUsersButton->setProperty("class", "secondary");
    copyUsersButton->setMinimumHeight(32);
    connect(copyUsersButton, &QPushButton::clicked, this, &LoginWindow::copyTestUsersToClipboard);
    copyLayout->addWidget(copyUsersButton);
    testLayout->addLayout(copyLayout);

    mainLayout->addWidget(testUsersPanel);

    errorLabel = new QLabel();
    errorLabel->setWordWrap(true);
    mainLayout->addWidget(errorLabel);

    mainLayout->addStretch();

    setMinimumSize(500, 700);
}

void LoginWindow::applyStyles() {
    loginButton->setObjectName("loginButton");
}

void LoginWindow::onLoginClicked() {
    QString email = emailInput->text().trimmed();
    QString password = passwordInput->text();

    if (email.isEmpty() || password.isEmpty()) {
        showError("Пожалуйста, заполните все поля");
        return;
    }

    qDebug() << "Login attempt for email:" << email;
    
    QString dataPath = QCoreApplication::applicationDirPath() + "/../data";
    DataManager dm(dataPath);
    qDebug() << "Using data path:" << dataPath;
    int userId = -1;
    int userType = -1;
    bool authenticated = false;

    qDebug() << "Checking patient...";
    if (dm.patientLoginByEmail(email, password)) {
        Patient patient = dm.getPatientByEmail(email);
        qDebug() << "Patient found:" << patient.id_patient << patient.email;
        if (patient.id_patient > 0) {
            userId = patient.id_patient;
            userType = 0;
            authenticated = true;
        }
    }
    
    if (!authenticated) {
        qDebug() << "Checking doctor...";
        if (dm.doctorLoginByEmail(email, password)) {
            Doctor doctor = dm.getDoctorByEmail(email);
            qDebug() << "Doctor found:" << doctor.id_doctor << doctor.email;
            if (doctor.id_doctor > 0) {
                userId = doctor.id_doctor;
                userType = 1;
                authenticated = true;
            }
        }
    }
    
    if (!authenticated) {
        qDebug() << "Checking manager...";
        if (dm.managerLoginByEmail(email, password)) {
            Manager manager = dm.getManagerByEmail(email);
            qDebug() << "Manager found:" << manager.id << manager.email;
            if (manager.id > 0) {
                userId = manager.id;
                userType = 2;
                authenticated = true;
            }
        }
    }

    if (!authenticated) {
        qDebug() << "Checking admin...";
        if (dm.adminLoginByEmail(email, password)) {
            Admin admin = dm.getAdminByEmail(email);
            qDebug() << "Admin found:" << admin.id << admin.email;
            if (admin.id > 0) {
                userId = admin.id;
                userType = 3;
                authenticated = true;
            }
        }
    }

    if (authenticated && userId > 0 && userType >= 0) {
        showSuccess("Успешный вход!");
        emit loginSuccess(userId, userType);
    } else {
        showError("Неверные учетные данные");
        qDebug() << "Authentication failed";
    }
}

void LoginWindow::onRegistrationClicked() {
    emit switchToRegistration();
}

void LoginWindow::onPasswordToggle() {
    if (passwordInput->echoMode() == QLineEdit::Password) {
        passwordInput->setEchoMode(QLineEdit::Normal);
        passwordToggleButton->setText("🙈");
    } else {
        passwordInput->setEchoMode(QLineEdit::Password);
        passwordToggleButton->setText("👁️");
    }
}

void LoginWindow::showError(const QString& message) {
    errorLabel->setText("⚠️ " + message);
    errorLabel->setProperty("class", "error-label");
    errorLabel->style()->unpolish(errorLabel);
    errorLabel->style()->polish(errorLabel);
}

void LoginWindow::showSuccess(const QString& message) {
    errorLabel->setText(message);
    errorLabel->setProperty("class", "success-label");
    errorLabel->style()->unpolish(errorLabel);
    errorLabel->style()->polish(errorLabel);
}

void LoginWindow::copyTestUsersToClipboard() {
    if (!testUsersText) return;
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) return;
    clipboard->setText(testUsersText->toPlainText());
    showSuccess("Данные тестовых пользователей скопированы в буфер обмена");
}
