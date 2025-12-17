#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "models.h"

class LoginWindow;
class RegistrationWindow;
class MainPage;

class QScrollArea;

class AuthWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();
    void reset();

signals:
    void loginSuccessful(const LoginUser &user);

private slots:
    void onLoginSuccess(int userId, int userType);
    void onSwitchToRegistration();
    void onSwitchToLogin();
    void onLogout();
    void onRegistrationSuccess();

private:
    void setupUI();
    void applyStyles();
    void updateScrollSize();

    QStackedWidget *stackedWidget;
    LoginWindow *loginWindow;
    RegistrationWindow *registrationWindow;
    MainPage *mainPage;
    QScrollArea *globalScroll;

    int currentUserId;
    int currentUserType;
};

#endif // AUTHWINDOW_H
