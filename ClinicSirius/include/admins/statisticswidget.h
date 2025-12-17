#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H

#include <QWidget>
#include <QDate>
#include <QVBoxLayout>

class DataManager;

#ifdef USE_QT_CHARTS
#include <QtCharts/QChartView>
#endif
class QLabel;
class QPushButton;

class StatisticsWidget : public QWidget {
    Q_OBJECT
public:
    explicit StatisticsWidget(DataManager *dm, QWidget *parent = nullptr);
    void refresh();

private:
    void buildUI();
    void buildWeeklyCharts();
    void buildTopLists();

    DataManager *dm;
    QWidget *visitsChartView;
    QWidget *doctorsChartView;
    QWidget *topPatientsChartView;
    QWidget *topDoctorsChartView;
    QLabel *periodLabel;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    QPushButton *choosePeriodBtn;
    QDate periodStart;
    QDate periodEnd;
    bool customPeriod = false;
    int endOffsetWeeks = 0; // 0 = up to this week, >0 = shift window to past
    QVBoxLayout *chartsLayout;

private slots:
    void moveChart(QWidget *w, int direction);
};

#endif // STATISTICSWIDGET_H
