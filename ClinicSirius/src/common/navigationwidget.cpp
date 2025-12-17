#include "navigationwidget.h"
#include <QLabel>

NavigationWidget::NavigationWidget(QWidget *parent)
    : QWidget(parent) {
    setupUI();
    applyStyles();
}

NavigationWidget::~NavigationWidget() {
}

void NavigationWidget::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    QLabel *logoLabel = new QLabel("Клиника «Сириус»");
    QFont logoFont = logoLabel->font();
    logoFont.setPointSize(14);
    logoFont.setBold(true);
    logoLabel->setFont(logoFont);
    layout->addWidget(logoLabel);

    layout->addSpacing(20);

    homeButton = new QPushButton("🏠 Главная");
    profileButton = new QPushButton("👤 Профиль");
    appointmentsButton = new QPushButton("📅 Приёмы");

    layout->addWidget(homeButton);
    layout->addWidget(profileButton);
    layout->addWidget(appointmentsButton);

    layout->addStretch();

    logoutButton = new QPushButton("🚪 Выход");
    layout->addWidget(logoutButton);

    connect(homeButton, &QPushButton::clicked, this, &NavigationWidget::homeClicked);
    connect(profileButton, &QPushButton::clicked, this, &NavigationWidget::profileClicked);
    connect(appointmentsButton, &QPushButton::clicked, this, &NavigationWidget::appointmentsClicked);
    connect(logoutButton, &QPushButton::clicked, this, &NavigationWidget::logoutClicked);
}

void NavigationWidget::applyStyles() {
    // Стили загружаются из styles.qss
}
