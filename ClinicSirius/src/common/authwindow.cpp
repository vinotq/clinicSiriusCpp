#include "authwindow.h"
#include "loginwindow.h"
#include "registrationwindow.h"
#include "mainpage.h"
#include "datamanager.h"
#include <QStackedWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QIcon>
#include <QCoreApplication>
#include <QTimer>

AuthWindow::AuthWindow(QWidget *parent)
    : QMainWindow(parent), currentUserId(-1), currentUserType(-1)
{
    setupUI();
    applyStyles();
    
    setWindowTitle("Клиника «Сириус»");
    setMinimumSize(1000, 700);
}

AuthWindow::~AuthWindow() {
}

void AuthWindow::setupUI() {
    // Создаем стек виджетов для переключения между окнами
    stackedWidget = new QStackedWidget();
    
    // Окно входа
    loginWindow = new LoginWindow();
    connect(loginWindow, &LoginWindow::loginSuccess, this, &AuthWindow::onLoginSuccess);
    connect(loginWindow, &LoginWindow::switchToRegistration, this, &AuthWindow::onSwitchToRegistration);
    stackedWidget->addWidget(loginWindow);
    
    // Окно регистрации
    registrationWindow = new RegistrationWindow();
    connect(registrationWindow, &RegistrationWindow::switchToLogin, this, &AuthWindow::onSwitchToLogin);
    connect(registrationWindow, &RegistrationWindow::registrationSuccess, this, &AuthWindow::onRegistrationSuccess);
    stackedWidget->addWidget(registrationWindow);
    
    // Главная страница
    mainPage = new MainPage();
    connect(mainPage, &MainPage::logoutRequested, this, &AuthWindow::onLogout);
    stackedWidget->addWidget(mainPage);
    
    // Wrap the stacked widget in a scroll area so the whole application can scroll
    globalScroll = new QScrollArea(this);
    globalScroll->setWidgetResizable(true);
    globalScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    globalScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // Make stackedWidget the child of the scroll area
    globalScroll->setWidget(stackedWidget);
    // Set the scroll area as the central widget
    setCentralWidget(globalScroll);
    
    // По умолчанию показываем окно входа
    stackedWidget->setCurrentWidget(loginWindow);
    QTimer::singleShot(0, this, &AuthWindow::updateScrollSize);
}

void AuthWindow::applyStyles() {
    // Стили загружаются из styles.qss
}

void AuthWindow::onLoginSuccess(int userId, int userType) {
    currentUserId = userId;
    currentUserType = userType;

    // Собираем данные пользователя для передачи дальше
    QString dataPath = QCoreApplication::applicationDirPath() + "/../data";
    DataManager dm(dataPath);
    LoginUser user;
    user.id = userId;

    switch (userType) {
        case 0: { // patient
            user.type = LoginUser::PATIENT;
            Patient p = dm.getPatientById(userId);
            user.name = p.fullName();
            break;
        }
        case 1: { // doctor
            user.type = LoginUser::DOCTOR;
            Doctor d = dm.getDoctorById(userId);
            user.name = d.fullName();
            break;
        }
        case 2: { // manager
            user.type = LoginUser::MANAGER;
            Manager m = dm.getManagerById(userId);
            user.name = m.fullName();
            break;
        }
        case 3: { // admin
            user.type = LoginUser::ADMIN;
            user.name = "Администратор";
            break;
        }
        default:
            user.type = LoginUser::PATIENT;
            user.name = "Пользователь";
            break;
    }

    emit loginSuccessful(user);

    // Переключаемся на главную страницу
    // Перед переключением передаём данные пользователя в mainPage
    mainPage->setCurrentUser(user);
    stackedWidget->setCurrentWidget(mainPage);
}

void AuthWindow::onSwitchToRegistration() {
    stackedWidget->setCurrentWidget(registrationWindow);
    updateScrollSize();
}

void AuthWindow::onSwitchToLogin() {
    stackedWidget->setCurrentWidget(loginWindow);
    updateScrollSize();
}

void AuthWindow::onLogout() {
    currentUserId = -1;
    currentUserType = -1;
    
    // Очищаем поля входа - берем emailInput напрямую из loginWindow
    // Переключаемся на окно входа
    stackedWidget->setCurrentWidget(loginWindow);
    updateScrollSize();
}

void AuthWindow::onRegistrationSuccess() {
    // Переключаемся на окно входа с сообщением об успехе
    stackedWidget->setCurrentWidget(loginWindow);
    updateScrollSize();
}

void AuthWindow::reset() {
    stackedWidget->setCurrentWidget(loginWindow);
    currentUserId = -1;
    currentUserType = -1;
    updateScrollSize();
}

void AuthWindow::updateScrollSize() {
    if (!stackedWidget) return;
    QWidget *current = stackedWidget->currentWidget();
    if (!current) return;
    // Используем sizeHint активной страницы как минимальный размер,
    // чтобы QScrollArea показывала скролл вместо сжатия содержимого.
    QSize hint = current->sizeHint();
    if (hint.isValid()) {
        stackedWidget->setMinimumSize(hint);
    }
}
