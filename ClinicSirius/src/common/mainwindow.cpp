#include "mainwindow.h"
#include "authwindow.h"
#include "mainpage.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), stackedWidget(nullptr), authWindow(nullptr), mainPage(nullptr) {
    setWindowTitle("Клиника «Сириус»");
    // Use vector SVG icon for crisp scaling
    // app icon removed; use emoji in UI where needed
    setGeometry(100, 100, 1280, 800);

    setupUI();
    connectSignals();

    // Start with auth window
    stackedWidget->setCurrentWidget(authWindow);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    authWindow = new AuthWindow(this);
    mainPage = new MainPage(this);

    stackedWidget->addWidget(authWindow);
    stackedWidget->addWidget(mainPage);
}

void MainWindow::connectSignals() {
    connect(authWindow, &AuthWindow::loginSuccessful, this, &MainWindow::onLoginSuccess);
    connect(mainPage, &MainPage::logoutRequested, this, &MainWindow::onLogout);
}

void MainWindow::onLoginSuccess(const LoginUser &user) {
    mainPage->setCurrentUser(user);
    stackedWidget->setCurrentWidget(mainPage);
}

void MainWindow::onLogout() {
    stackedWidget->setCurrentWidget(authWindow);
    authWindow->reset();
}
