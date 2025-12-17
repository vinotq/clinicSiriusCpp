#ifndef DETAILMENU_H
#define DETAILMENU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>

class DetailMenu : public QWidget {
    Q_OBJECT

public:
    explicit DetailMenu(QWidget *parent = nullptr);
    ~DetailMenu() = default;

    void addItem(const QString &label, const QString &value, const QString &icon = "");

    void addCustomItem(const QString &label, QWidget *widget);

    void clearItems();

    void showMenu();

    void hideMenu();

    void toggleMenu();

    bool isVisible() const;

    void setMenuWidth(int width);

signals:
    void menuOpenedChanged(bool opened);

private:
    void setupUI();
    void setupAnimation();
    void connectSignals();

    QWidget *menuContent;
    QVBoxLayout *menuLayout;
    QPushButton *toggleButton;
    QPropertyAnimation *slideAnimation;
    bool isOpen;
    int menuWidth;
};

#endif
