#include "scrollabletab.h"
#include <QScrollBar>

ScrollableTab::ScrollableTab(QWidget *parent)
    : QWidget(parent), scrollArea(nullptr), contentWidget(nullptr),
      mainLayout(nullptr), contentLayout_(nullptr) {
    setupUI();
    setupStyles();
}

void ScrollableTab::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Создаем QScrollArea
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setProperty("class", "scrollable-area");

    // Содержимое
    contentWidget = new QWidget();
    contentWidget->setProperty("class", "scrollable-content");
    contentLayout_ = new QVBoxLayout(contentWidget);
    contentLayout_->setContentsMargins(20, 20, 20, 20);
    contentLayout_->setSpacing(16);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}

void ScrollableTab::setupStyles() {
    // Scrollbar styling moved to resources/styles.qss
}

void ScrollableTab::addContent(QWidget *widget) {
    if (widget) {
        contentLayout_->addWidget(widget);
    }
}

void ScrollableTab::addStretchedContent(QWidget *widget, int stretch) {
    if (widget) {
        contentLayout_->addWidget(widget, stretch);
    }
}

void ScrollableTab::addStretch(int stretch) {
    contentLayout_->addStretch(stretch);
}

QVBoxLayout *ScrollableTab::contentLayout() const {
    return contentLayout_;
}

void ScrollableTab::clearContent() {
    while (QLayoutItem *item = contentLayout_->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void ScrollableTab::setContentMargins(int left, int top, int right, int bottom) {
    contentLayout_->setContentsMargins(left, top, right, bottom);
}

void ScrollableTab::setContentSpacing(int spacing) {
    contentLayout_->setSpacing(spacing);
}

void ScrollableTab::scrollToTop() {
    scrollArea->verticalScrollBar()->setValue(0);
}

void ScrollableTab::scrollToBottom() {
    scrollArea->verticalScrollBar()->setValue(
        scrollArea->verticalScrollBar()->maximum()
    );
}
