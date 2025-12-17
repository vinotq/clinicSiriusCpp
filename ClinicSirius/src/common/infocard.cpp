#include "infocard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>

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
    iconLabel->setProperty("class", "info-icon");
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
    QString resolved = iconPath;
    QString key = iconPath.toLower();

    if (resolved.isEmpty()) {
        resolved = ":/images/icon-info.svg";
    }

    if (!resolved.startsWith(":/")) {
        if (key.contains("clinic") || key.contains("sirius")) resolved = ":/images/clinic.svg";
        else if (key.contains("doctor") || key.contains("medical")) resolved = ":/images/icon-service-doctor.svg";
        else if (key.contains("check")) resolved = ":/images/icon-check.svg";
        else if (key.contains("cancel") || key.contains("delete")) resolved = ":/images/icon-trash.svg";
        else if (key.contains("eye-off")) resolved = ":/images/icon-eye-off.svg";
        else if (key.contains("eye") || key.contains("visibility")) resolved = ":/images/icon-eye.svg";
        else if (key.contains("profile") || key.contains("account")) resolved = ":/images/icon-user.svg";
        else if (key.contains("location")) resolved = ":/images/icon-pin.svg";
        else if (key.contains("phone") || key.contains("call")) resolved = ":/images/icon-phone.svg";
        else if (key.contains("history") || key.contains("autorenew")) resolved = ":/images/icon-refresh.svg";
        else if (key.contains("clock") || key.contains("schedule")) resolved = ":/images/icon-clock.svg";
        else resolved = ":/images/icon-info.svg";
    }

    QPixmap pm(resolved);
    if (!pm.isNull()) {
        iconLabel->setPixmap(pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLabel->setText("");
    } else {
        iconLabel->clear();
    }
}
