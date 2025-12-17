#include "detailmenu.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

DetailMenu::DetailMenu(QWidget *parent)
    : QWidget(parent), menuContent(nullptr), menuLayout(nullptr),
      toggleButton(nullptr), slideAnimation(nullptr), isOpen(false), menuWidth(300) {
    setupUI();
    setupAnimation();
    connectSignals();
}

void DetailMenu::setupUI() {
    // Основной layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Кнопка-переключатель
    toggleButton = new QPushButton(this);
    toggleButton->setText("ℹ");  // Иконка информации
    toggleButton->setMaximumWidth(40);
    toggleButton->setProperty("class", "detail-toggle");

    // Контейнер меню
    menuContent = new QWidget(this);
    menuContent->setMaximumWidth(menuWidth);
    menuContent->setMinimumWidth(0);
    menuContent->setProperty("class", "detail-menu");

    // Layout меню
    menuLayout = new QVBoxLayout(menuContent);
    menuLayout->setContentsMargins(16, 16, 16, 16);
    menuLayout->setSpacing(12);
    menuLayout->addStretch();

    mainLayout->addWidget(menuContent);
    mainLayout->addWidget(toggleButton);

    // Скрываем меню в начале
    menuContent->setMaximumWidth(0);
}

void DetailMenu::setupAnimation() {
    slideAnimation = new QPropertyAnimation(menuContent, "maximumWidth", this);
    slideAnimation->setDuration(300);
    slideAnimation->setEasingCurve(QEasingCurve::InOutQuart);
}

void DetailMenu::connectSignals() {
    connect(toggleButton, &QPushButton::clicked, this, &DetailMenu::toggleMenu);
}

void DetailMenu::addItem(const QString &label, const QString &value, const QString &icon) {
    // Удаляем растяжение для добавления элемента
    if (menuLayout->count() > 0) {
        QLayoutItem *last = menuLayout->itemAt(menuLayout->count() - 1);
        if (last->spacerItem()) {
            menuLayout->takeAt(menuLayout->count() - 1);
        }
    }

    // Контейнер для элемента
    QWidget *itemWidget = new QWidget();
    QVBoxLayout *itemLayout = new QVBoxLayout(itemWidget);
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->setSpacing(4);

    // Подпись
    QLabel *labelWidget = new QLabel(label);
    labelWidget->setProperty("class", "detail-label");

    // Значение
    QLabel *valueWidget = new QLabel(value);
    valueWidget->setWordWrap(true);
    valueWidget->setProperty("class", "detail-value");

    itemLayout->addWidget(labelWidget);
    itemLayout->addWidget(valueWidget);

    menuLayout->addWidget(itemWidget);
    menuLayout->addSpacing(8);

    // Добавляем растяжение в конец
    menuLayout->addStretch();
}

void DetailMenu::addCustomItem(const QString &label, QWidget *widget) {
    // Удаляем растяжение
    if (menuLayout->count() > 0) {
        QLayoutItem *last = menuLayout->itemAt(menuLayout->count() - 1);
        if (last->spacerItem()) {
            menuLayout->takeAt(menuLayout->count() - 1);
        }
    }

    QWidget *itemWidget = new QWidget();
    QVBoxLayout *itemLayout = new QVBoxLayout(itemWidget);
    itemLayout->setContentsMargins(0, 0, 0, 0);
    itemLayout->setSpacing(4);

    QLabel *labelWidget = new QLabel(label);
    labelWidget->setProperty("class", "detail-label");

    itemLayout->addWidget(labelWidget);
    itemLayout->addWidget(widget);

    menuLayout->addWidget(itemWidget);
    menuLayout->addSpacing(8);

    menuLayout->addStretch();
}

void DetailMenu::clearItems() {
    while (QLayoutItem *item = menuLayout->takeAt(0)) {
        if (item->widget() && item->widget() != nullptr) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    menuLayout->addStretch();
}

void DetailMenu::showMenu() {
    if (isOpen) return;

    isOpen = true;
    slideAnimation->setStartValue(0);
    slideAnimation->setEndValue(menuWidth);
    slideAnimation->start();

    emit menuOpenedChanged(true);
}

void DetailMenu::hideMenu() {
    if (!isOpen) return;

    isOpen = false;
    slideAnimation->setStartValue(menuWidth);
    slideAnimation->setEndValue(0);
    slideAnimation->start();

    emit menuOpenedChanged(false);
}

void DetailMenu::toggleMenu() {
    if (isOpen) {
        hideMenu();
    } else {
        showMenu();
    }
}

bool DetailMenu::isVisible() const {
    return isOpen;
}

void DetailMenu::setMenuWidth(int width) {
    menuWidth = width;
    if (isOpen) {
        menuContent->setMaximumWidth(width);
    }
}
