#include "styledbuttongroup.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

StyledButtonGroup::StyledButtonGroup(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent), orientation(orientation) {
    if (orientation == Qt::Horizontal) {
        new QHBoxLayout(this);
    } else {
        new QVBoxLayout(this);
    }
    layout()->setContentsMargins(0, 0, 0, 0);
    applyStyles();
}

StyledButtonGroup::~StyledButtonGroup() {
}

void StyledButtonGroup::addButton(const QString &text, const QString &objectName) {
    QPushButton *button = new QPushButton(text);
    if (!objectName.isEmpty()) {
        button->setObjectName(objectName);
    }
    
    int index = buttons.size();
    buttons.append(button);
    
    layout()->addWidget(button);
    
    connect(button, &QPushButton::clicked, this, [this, index, text]() {
        emit buttonClicked(index, text);
    });
}

QPushButton *StyledButtonGroup::getButton(int index) {
    if (index >= 0 && index < buttons.size()) {
        return buttons[index];
    }
    return nullptr;
}

int StyledButtonGroup::buttonCount() const {
    return buttons.size();
}

void StyledButtonGroup::applyStyles() {
    // Стили загружаются из styles.qss
}
