#include "infocard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>

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
    // Replace image icons with emoji equivalents. Map common icon names
    // to emoji and set as label text. This avoids loading image resources.
    QString key = iconPath.toLower();
    QString emoji = "🔹";
    if (key.contains("clinic") || key.contains("sirius")) emoji = "🏥";
    else if (key.contains("doctor") || key.contains("medical")) emoji = QString::fromUtf8("👩\u200D⚕️");
    else if (key.contains("check")) emoji = "✅";
    else if (key.contains("cancel") || key.contains("delete")) emoji = "🗑";
    else if (key.contains("eye-off")) emoji = "🙈";
    else if (key.contains("eye") || key.contains("visibility")) emoji = "👁️";
    else if (key.contains("profile") || key.contains("account")) emoji = "👤";
    else if (key.contains("location")) emoji = "📍";
    else if (key.contains("phone") || key.contains("call")) emoji = "📞";
    else if (key.contains("history") || key.contains("autorenew")) emoji = "🕘";
    else if (key.contains("clock") || key.contains("schedule")) emoji = "⏰";

    iconLabel->setText(emoji);
    QFont f = iconLabel->font();
    f.setPointSize(24);
    iconLabel->setFont(f);
}
