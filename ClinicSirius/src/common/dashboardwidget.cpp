#include "dashboardwidget.h"
#include <QGroupBox>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent) {
    setupUI();
    applyStyles();
}

DashboardWidget::~DashboardWidget() {
}

void DashboardWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    welcomeLabel = new QLabel();
    QFont font = welcomeLabel->font();
    font.setPointSize(16);
    font.setBold(true);
    welcomeLabel->setFont(font);
    mainLayout->addWidget(welcomeLabel);

    QGroupBox *statsGroup = new QGroupBox("Statistics");
    statisticsLayout = new QVBoxLayout(statsGroup);
    mainLayout->addWidget(statsGroup);

    mainLayout->addStretch();
}

void DashboardWidget::applyStyles() {
    setProperty("class", "dashboard-widget");
}

void DashboardWidget::setWelcomeMessage(const QString &message) {
    welcomeLabel->setText(message);
}

void DashboardWidget::addStatistic(const QString &title, const QString &value) {
    QHBoxLayout *statLayout = new QHBoxLayout();
    
    QLabel *titleLabel = new QLabel(title + ":");
    titleLabel->setMinimumWidth(150);
    
    QLabel *valueLabel = new QLabel(value);
    QFont font = valueLabel->font();
    font.setBold(true);
    valueLabel->setFont(font);
    
    statLayout->addWidget(titleLabel);
    statLayout->addWidget(valueLabel);
    statLayout->addStretch();
    
    statisticsLayout->addLayout(statLayout);
}
