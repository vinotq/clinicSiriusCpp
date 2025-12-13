#ifndef STYLEDBUTTONGROUP_H
#define STYLEDBUTTONGROUP_H

#include <QWidget>
#include <QPushButton>
#include <QVector>

class StyledButtonGroup : public QWidget {
    Q_OBJECT

public:
    explicit StyledButtonGroup(Qt::Orientation orientation = Qt::Horizontal,
                              QWidget *parent = nullptr);
    ~StyledButtonGroup();

    void addButton(const QString &text, const QString &objectName = "");
    QPushButton *getButton(int index);
    int buttonCount() const;

signals:
    void buttonClicked(int index, const QString &text);

private:
    void applyStyles();

    Qt::Orientation orientation;
    QVector<QPushButton*> buttons;
};

#endif // STYLEDBUTTONGROUP_H
