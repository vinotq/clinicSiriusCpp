#include "statsbadge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QEnterEvent>

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
    iconLabel->setStyleSheet(
        "background-color: #e0e7ff; border-radius: 8px; "
        "display: flex; align-items: center; justify-content: center;"
    );

    // Значение
    valueLabel = new QLabel("0");
    valueLabel->setStyleSheet(
        "font-size: 28pt; font-weight: bold; color: #1f2937;"
    );

    topLayout->addWidget(iconLabel);
    topLayout->addWidget(valueLabel);
    topLayout->addStretch();

    // Подпись
    labelLabel = new QLabel("Метрика");
    labelLabel->setStyleSheet(
        "color: #6b7280; font-size: 9pt; font-weight: 600; text-transform: uppercase;"
    );

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(labelLabel);

    // Стилизация контейнера
    setStyleSheet(
        "QWidget {"
        "  background-color: white;"
        "  border: 1px solid #e5e7eb;"
        "  border-radius: 10px;"
        "}"
        "QWidget:hover {"
        "  border: 1px solid #d1d5db;"
        "  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);"
        "}"
    );

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
        QPixmap pixmap(iconPath);
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaledToWidth(24, Qt::SmoothTransformation);
            iconLabel->setPixmap(pixmap);
        }
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
    QString bgColor;
    QString borderColor;

    switch (badgeType) {
        case Primary:
            bgColor = "#e0e7ff";
            borderColor = "#c7d2fe";
            valueLabel->setStyleSheet(
                "font-size: 28pt; font-weight: bold; color: #3b82f6;"
            );
            break;
        case Success:
            bgColor = "#d1fae5";
            borderColor = "#a7f3d0";
            valueLabel->setStyleSheet(
                "font-size: 28pt; font-weight: bold; color: #10b981;"
            );
            break;
        case Warning:
            bgColor = "#fef3c7";
            borderColor = "#fde68a";
            valueLabel->setStyleSheet(
                "font-size: 28pt; font-weight: bold; color: #f59e0b;"
            );
            break;
        case Danger:
            bgColor = "#fee2e2";
            borderColor = "#fecaca";
            valueLabel->setStyleSheet(
                "font-size: 28pt; font-weight: bold; color: #ef4444;"
            );
            break;
        case Info:
            bgColor = "#cffafe";
            borderColor = "#a5f3fc";
            valueLabel->setStyleSheet(
                "font-size: 28pt; font-weight: bold; color: #06b6d4;"
            );
            break;
    }

    iconLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 8px;").arg(bgColor)
    );
}

void StatsBadge::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
    setStyleSheet(
        "QWidget {"
        "  background-color: #f9fafb;"
        "  border: 1px solid #d1d5db;"
        "  border-radius: 10px;"
        "  box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);"
        "}"
    );
}

void StatsBadge::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    setStyleSheet(
        "QWidget {"
        "  background-color: white;"
        "  border: 1px solid #e5e7eb;"
        "  border-radius: 10px;"
        "}"
    );
}
