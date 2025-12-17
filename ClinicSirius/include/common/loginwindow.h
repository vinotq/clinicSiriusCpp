#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>

class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccess(int userId, int userType);
    void switchToRegistration();

private slots:
    void onLoginClicked();
    void onRegistrationClicked();
    void onPasswordToggle();
    void copyTestUsersToClipboard();

private:
    void setupUI();
    void applyStyles();
    void showError(const QString& message);
    void showSuccess(const QString& message);

    QLineEdit *emailInput;
    QLineEdit *passwordInput;
    QPushButton *loginButton;
    QPushButton *passwordToggleButton;
    QPushButton *registrationButton;
    QPushButton *copyUsersButton;
    QLabel *errorLabel;
    QPlainTextEdit *testUsersText;
};

#endif
