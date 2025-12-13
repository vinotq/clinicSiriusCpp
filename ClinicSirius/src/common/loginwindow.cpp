#include "loginwindow.h"
#include "datamanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QMessageBox>
#include <QFont>
#include <QLabel>
#include <QDebug>
#include <QCoreApplication>
#include <QStyle>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    applyStyles();
}

LoginWindow::~LoginWindow() {
}

void LoginWindow::setupUI() {
    // Основной лейаут
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // Лого/Заголовок
    QLabel *logoLabel = new QLabel();
    logoLabel->setText("🏥 Clinic Sirius");
    QFont logoFont;
    logoFont.setPointSize(24);
    logoFont.setBold(true);
    logoLabel->setFont(logoFont);
    logoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(logoLabel);

    // Приветствие
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

    // Email/Username поле
    QLabel *emailLabel = new QLabel("Email");
    QFont fieldLabelFont;
    fieldLabelFont.setPointSize(10);
    emailLabel->setFont(fieldLabelFont);
    mainLayout->addWidget(emailLabel);

    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("Введите email");
    emailInput->setMinimumHeight(40);
    mainLayout->addWidget(emailInput);

    // Пароль поле
    QLabel *passwordLabel = new QLabel("Пароль");
    passwordLabel->setFont(fieldLabelFont);
    mainLayout->addWidget(passwordLabel);

    QHBoxLayout *passwordLayout = new QHBoxLayout();
    passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("Введите пароль");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setMinimumHeight(40);
    passwordLayout->addWidget(passwordInput);

    passwordToggleButton = new QPushButton("👁");
    passwordToggleButton->setMaximumWidth(45);
    passwordToggleButton->setMinimumHeight(40);
    connect(passwordToggleButton, &QPushButton::clicked, this, &LoginWindow::onPasswordToggle);
    passwordLayout->addWidget(passwordToggleButton);

    mainLayout->addLayout(passwordLayout);

    mainLayout->addSpacing(10);

    // Забыли пароль
    QPushButton *forgotButton = new QPushButton("Забыли пароль?");
    forgotButton->setFlat(true);
    forgotButton->setProperty("class", "login-forgot");
    mainLayout->addWidget(forgotButton, 0, Qt::AlignRight);

    mainLayout->addSpacing(10);

    // Кнопка входа
    loginButton = new QPushButton("Войти");
    loginButton->setMinimumHeight(45);
    loginButton->setFont(QFont("Arial", 12, QFont::Bold));
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    mainLayout->addWidget(loginButton);

    // Разделител
    QLabel *orLabel = new QLabel("или");
    orLabel->setAlignment(Qt::AlignCenter);
    orLabel->setProperty("class", "login-separator");
    mainLayout->addWidget(orLabel);

    // Кнопка регистрации
    QHBoxLayout *registrationLayout = new QHBoxLayout();
    QLabel *noAccountLabel = new QLabel("Нет аккаунта?");
    registrationButton = new QPushButton("Зарегистрируйтесь");
    registrationButton->setFlat(true);
    registrationButton->setProperty("class", "login-forgot");
    connect(registrationButton, &QPushButton::clicked, this, &LoginWindow::onRegistrationClicked);
    registrationLayout->addStretch();
    registrationLayout->addWidget(noAccountLabel);
    registrationLayout->addWidget(registrationButton);
    registrationLayout->addStretch();
    mainLayout->addLayout(registrationLayout);

    // Сообщение об ошибке
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
    
    // Проверка учетных данных через DataManager
    QString dataPath = QCoreApplication::applicationDirPath() + "/../data";
    DataManager dm(dataPath);
    qDebug() << "Using data path:" << dataPath;
    int userId = -1;
    int userType = -1;
    bool authenticated = false;

    // Попытка входа для пациента
    qDebug() << "Checking patient...";
    if (dm.patientLoginByEmail(email, password)) {
        Patient patient = dm.getPatientByEmail(email);
        qDebug() << "Patient found:" << patient.id_patient << patient.email;
        if (patient.id_patient > 0) {
            userId = patient.id_patient;
            userType = 0; // Patient
            authenticated = true;
        }
    }
    
    // Попытка входа для врача
    if (!authenticated) {
        qDebug() << "Checking doctor...";
        if (dm.doctorLoginByEmail(email, password)) {
            Doctor doctor = dm.getDoctorByEmail(email);
            qDebug() << "Doctor found:" << doctor.id_doctor << doctor.email;
            if (doctor.id_doctor > 0) {
                userId = doctor.id_doctor;
                userType = 1; // Doctor
                authenticated = true;
            }
        }
    }
    
    // Попытка входа для менеджера
    if (!authenticated) {
        qDebug() << "Checking manager...";
        if (dm.managerLoginByEmail(email, password)) {
            Manager manager = dm.getManagerByEmail(email);
            qDebug() << "Manager found:" << manager.id << manager.email;
            if (manager.id > 0) {
                userId = manager.id;
                userType = 2; // Manager
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
        passwordToggleButton->setText("👁");
    }
}

void LoginWindow::showError(const QString& message) {
    errorLabel->setText(message);
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
