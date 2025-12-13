#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class DashboardWidget : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);
    ~DashboardWidget();

    void setWelcomeMessage(const QString &message);
    void addStatistic(const QString &title, const QString &value);

private:
    void setupUI();
    void applyStyles();

    QLabel *welcomeLabel;
    QVBoxLayout *statisticsLayout;
};

#endif // DASHBOARDWIDGET_H
