#include "statsbadge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QEnterEvent>
#include <QStyle>
#include <QIcon>

StatsBadge::StatsBadge(QWidget *parent)
    : QWidget(parent), valueLabel(nullptr), labelLabel(nullptr),
      iconLabel(nullptr), badgeType(Primary), currentValue(0) {
    setupUI();
    setMinimumHeight(120);
    setMaximumHeight(160);
}

void StatsBadge::setupUI() {
    // Основной layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 12, 16, 12);
    mainLayout->setSpacing(8);

    // Верхняя часть (иконка + значение)
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(12);

    // Иконка
    iconLabel = new QLabel();
    iconLabel->setMinimumSize(40, 40);
    iconLabel->setMaximumSize(40, 40);
    iconLabel->setProperty("class", "stats-icon");

    // Значение
    valueLabel = new QLabel("0");
    valueLabel->setProperty("class", "stats-value");

    topLayout->addWidget(iconLabel);
    topLayout->addWidget(valueLabel);
    topLayout->addStretch();

    // Подпись
    labelLabel = new QLabel("Метрика");
    labelLabel->setProperty("class", "stats-label");

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(labelLabel);

    // Используем классы и свойства — стили определены в resources/styles.qss
    setProperty("class", "stats-badge");
    updateStyle();
}

void StatsBadge::setValue(int value) {
    currentValue = value;
    valueLabel->setText(QString::number(value));
}

void StatsBadge::setLabel(const QString &label) {
    labelLabel->setText(label);
}

void StatsBadge::setIcon(const QString &iconPath) {
    if (!iconPath.isEmpty()) {
            QString key = iconPath.toLower();
            QString emoji = "🔹";
            if (key.contains("up")) emoji = "⬆️";
            else if (key.contains("down")) emoji = "⬇️";
            else if (key.contains("doctor") || key.contains("medical")) emoji = QString::fromUtf8("👩\u200D⚕️");
            else if (key.contains("check")) emoji = "✅";
            else if (key.contains("cancel") || key.contains("delete")) emoji = "🗑";
            iconLabel->setText(emoji);
            QFont f = iconLabel->font(); f.setPointSize(12); iconLabel->setFont(f);
    }
}

void StatsBadge::setBadgeType(BadgeType type) {
    badgeType = type;
    updateStyle();
}

void StatsBadge::setDescription(const QString &desc) {
    description = desc;
    setToolTip(description);
}

int StatsBadge::getValue() const {
    return currentValue;
}

void StatsBadge::updateStyle() {
    // Обновляем свойства типов бейджа — QSS будет их обрабатывать
    QString typeName;
    switch (badgeType) {
        case Primary: typeName = "primary"; break;
        case Success: typeName = "success"; break;
        case Warning: typeName = "warning"; break;
        case Danger: typeName = "danger"; break;
        case Info: typeName = "info"; break;
    }
    setProperty("badgeType", typeName);
    iconLabel->setProperty("badgeType", typeName);
    valueLabel->setProperty("badgeType", typeName);
    // force style update
    style()->unpolish(this);
    style()->polish(this);
}

void StatsBadge::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
    // Hover handled by QSS selectors for [class="stats-badge"]:hover
}

void StatsBadge::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    // Hover handled by QSS selectors for [class="stats-badge"]
}
