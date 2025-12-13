#include "infocard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

InfoCard::InfoCard(const QString &title, const QString &description,
                  const QString &iconPath, QWidget *parent)
    : QWidget(parent) {
    setupUI();
    setTitle(title);
    setDescription(description);
    if (!iconPath.isEmpty()) {
        setIcon(iconPath);
    }
    applyStyles();
}

InfoCard::~InfoCard() {
}

void InfoCard::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    iconLabel = new QLabel();
    iconLabel->setFixedSize(60, 60);
    iconLabel->setAlignment(Qt::AlignCenter);

    titleLabel = new QLabel();
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    descriptionLabel = new QLabel();
    descriptionLabel->setWordWrap(true);
    QFont descFont = descriptionLabel->font();
    descFont.setPointSize(10);
    descriptionLabel->setFont(descFont);

    mainLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(descriptionLabel);
}

void InfoCard::applyStyles() {
    setProperty("class", "info-card");
}

void InfoCard::setTitle(const QString &title) {
    titleLabel->setText(title);
}

void InfoCard::setDescription(const QString &description) {
    descriptionLabel->setText(description);
}

void InfoCard::setIcon(const QString &iconPath) {
    QPixmap pixmap(iconPath);
    if (!pixmap.isNull()) {
        iconLabel->setPixmap(pixmap.scaledToWidth(60, Qt::SmoothTransformation));
    }
}
