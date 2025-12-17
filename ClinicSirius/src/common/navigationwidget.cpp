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

    homeButton = new QPushButton("Главная");
    homeButton->setIcon(QIcon(":/images/icon-info.svg"));
    homeButton->setIconSize(QSize(18,18));
    homeButton->setProperty("class", "nav-button");
    profileButton = new QPushButton("Профиль");
    profileButton->setIcon(QIcon(":/images/icon-user.svg"));
    profileButton->setIconSize(QSize(18,18));
    profileButton->setProperty("class", "nav-button");
    appointmentsButton = new QPushButton("Приёмы");
    appointmentsButton->setIcon(QIcon(":/images/icon-calendar.svg"));
    appointmentsButton->setIconSize(QSize(18,18));
    appointmentsButton->setProperty("class", "nav-button");

    layout->addWidget(homeButton);
    layout->addWidget(profileButton);
    layout->addWidget(appointmentsButton);

    layout->addStretch();

    logoutButton = new QPushButton("Выход");
    logoutButton->setIcon(QIcon(":/images/icon-close.svg"));
    logoutButton->setIconSize(QSize(18,18));
    logoutButton->setProperty("class", "nav-button");
    layout->addWidget(logoutButton);

    connect(homeButton, &QPushButton::clicked, this, &NavigationWidget::homeClicked);
    connect(profileButton, &QPushButton::clicked, this, &NavigationWidget::profileClicked);
    connect(appointmentsButton, &QPushButton::clicked, this, &NavigationWidget::appointmentsClicked);
    connect(logoutButton, &QPushButton::clicked, this, &NavigationWidget::logoutClicked);
}

void NavigationWidget::applyStyles() {
    // Стили загружаются из styles.qss
}
