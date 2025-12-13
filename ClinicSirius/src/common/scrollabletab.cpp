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
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; background-color: #ffffff; }"
    );

    // Содержимое
    contentWidget = new QWidget();
    contentWidget->setStyleSheet("background-color: #ffffff;");
    contentLayout_ = new QVBoxLayout(contentWidget);
    contentLayout_->setContentsMargins(20, 20, 20, 20);
    contentLayout_->setSpacing(16);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}

void ScrollableTab::setupStyles() {
    // Стилизация scrollbar
    scrollArea->verticalScrollBar()->setStyleSheet(
        "QScrollBar:vertical {"
        "  border: none;"
        "  background: #f5f5f5;"
        "  width: 8px;"
        "  margin: 0px 0px 0px 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #cbd5e0;"
        "  border-radius: 4px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #a0aec0;"
        "}"
        "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical {"
        "  border: none;"
        "  background: none;"
        "}"
    );
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
