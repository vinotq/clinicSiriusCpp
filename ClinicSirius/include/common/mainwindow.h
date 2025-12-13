#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "models.h"

class AuthWindow;
class MainPage;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginSuccess(const LoginUser &user);
    void onLogout();

private:
    void setupUI();
    void connectSignals();

    QStackedWidget *stackedWidget;
    AuthWindow *authWindow;
    MainPage *mainPage;
};

#endif // MAINWINDOW_H
