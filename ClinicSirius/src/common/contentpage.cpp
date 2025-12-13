#include "contentpage.h"

ContentPage::ContentPage(QWidget *parent)
    : QWidget(parent) {
    setupUI();
    applyStyles();
}

ContentPage::~ContentPage() {
}

void ContentPage::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    titleLabel = new QLabel("");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(20);

    contentLayout = new QVBoxLayout();
    mainLayout->addLayout(contentLayout, 1);
}

void ContentPage::applyStyles() {
    // Стили загружаются из styles.qss
}

void ContentPage::setTitle(const QString &title) {
    titleLabel->setText(title);
}

void ContentPage::setContent(QWidget *widget) {
    if (contentLayout->count() > 0) {
        QLayoutItem *item = contentLayout->takeAt(0);
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    contentLayout->addWidget(widget);
}
