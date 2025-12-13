#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class RegistrationWindow : public QWidget {
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget *parent = nullptr);
    ~RegistrationWindow();

signals:
    void registrationSuccess();
    void switchToLogin();

private slots:
    void onRegisterClicked();
    void onLoginClicked();
    void onPasswordToggle();
    void onConfirmPasswordToggle();
    void validatePasswords();

private:
    void setupUI();
    void applyStyles();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    bool validateInput();

    QLineEdit *emailInput;
    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QLineEdit *confirmPasswordInput;
    QPushButton *registerButton;
    QPushButton *passwordToggleButton;
    QPushButton *confirmPasswordToggleButton;
    QPushButton *loginButton;
    QLabel *errorLabel;
    QLabel *passwordMatchLabel;
};

#endif // REGISTRATIONWINDOW_H
