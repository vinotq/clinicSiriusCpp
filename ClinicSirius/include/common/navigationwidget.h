#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

class NavigationWidget : public QWidget {
    Q_OBJECT

public:
    explicit NavigationWidget(QWidget *parent = nullptr);
    ~NavigationWidget();

signals:
    void homeClicked();
    void profileClicked();
    void appointmentsClicked();
    void logoutClicked();

private:
    void setupUI();
    void applyStyles();

    QPushButton *homeButton;
    QPushButton *profileButton;
    QPushButton *appointmentsButton;
    QPushButton *logoutButton;
};

#endif // NAVIGATIONWIDGET_H
