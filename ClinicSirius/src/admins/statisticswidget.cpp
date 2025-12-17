#include "admins/statisticswidget.h"
#include "../common/datamanager.h"
#include "../common/models.h"
#ifdef USE_QT_CHARTS
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QXYSeries>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QtCharts/QtCharts>
#endif
#include <QDateTime>
#include <QMap>
#include <QColor>
#include <functional>
#include <QScrollArea>
#include <QPainter>
#include <QToolTip>
#include <QCursor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QSize>
#include <QDateEdit>
#include <QDialog>
#include <QSizeGrip>
#include <QFrame>
#include <QSpacerItem>
#include <QTableWidget>
#include <algorithm>
#include <QSizeGrip>
#include <QMouseEvent>
static QString medicalPrimaryColor() { return "#0b6fa4"; }
static QString medicalAccentColor() { return "#2aa198"; }

// Format short person name: Фамилия И.О (id)
static QString formatShortPerson(const QString &lname, const QString &fname, const QString &tname, int id) {
    QString res = lname;
    if (!fname.isEmpty()) res += " " + fname.left(1) + ".";
    if (!tname.isEmpty()) res += tname.left(1) + ".";
    res += QString(" (%1)").arg(id);
    return res;
}


// Small helper widget that wraps a content widget and provides a corner size grip.
// Small handle that resizes only the target widget (the chart view), not the whole window
class ResizeHandle : public QWidget {
public:
    ResizeHandle(QWidget *target, QWidget *parent = nullptr)
        : QWidget(parent), target(target), pressed(false) {
        setFixedSize(14, 14);
        setCursor(Qt::SizeVerCursor);
    }
protected:
    void paintEvent(QPaintEvent *e) override {
        Q_UNUSED(e)
        QPainter p(this);
        p.fillRect(rect(), QColor(200,200,200));
    }
    void mousePressEvent(QMouseEvent *e) override {
        pressed = true;
        lastY = e->globalPosition().y();
        e->accept();
    }
    void mouseMoveEvent(QMouseEvent *e) override {
        if (!pressed || !target) return;
        qreal gy = e->globalPosition().y();
        int dy = int(gy - lastY);
        lastY = gy;
        int curH = target->height();
        int newH = qMax(target->minimumHeight(), curH + dy);
        target->setMinimumHeight(newH);
        target->resize(target->width(), newH);
        target->updateGeometry();
        // inform wrapper to expand so the outer layout and scrollarea grow
        QWidget *wrap = this->parentWidget();
        if (wrap) {
            int extra = wrap->height() - target->height();
            if (extra < 0) extra = 0;
            int newTotal = newH + extra;
            wrap->setMinimumHeight(newTotal);
            wrap->resize(wrap->width(), newTotal);
            wrap->updateGeometry();
        }
    }
    void mouseReleaseEvent(QMouseEvent *e) override {
        Q_UNUSED(e)
        pressed = false;
    }
private:
    QWidget *target;
    bool pressed;
    qreal lastY;
};

class ResizableWidget : public QFrame {
public:
    // onMove callback: direction = -1 (up), +1 (down)
    ResizableWidget(QWidget *content, bool allowShrink = true, const QString &title = QString(), QWidget *parent = nullptr)
        : QFrame(parent), contentWidget(content), allowShrink(allowShrink) {
        this->setFrameShape(QFrame::StyledPanel);
        this->setLineWidth(1);
        QVBoxLayout *l = new QVBoxLayout(this);
        l->setContentsMargins(4,4,4,4);

        // Header with title and move buttons
        QHBoxLayout *header = new QHBoxLayout();
        titleLabel = new QLabel(title);
        header->addWidget(titleLabel);
        header->addStretch();
        upBtn = new QPushButton();
        downBtn = new QPushButton();
        upBtn->setIcon(QIcon(":/images/icon-arrow-up.svg"));
        upBtn->setIconSize(QSize(14,14));
        upBtn->setToolTip("Переместить выше");
        downBtn->setIcon(QIcon(":/images/icon-arrow-down.svg"));
        downBtn->setIconSize(QSize(14,14));
        downBtn->setToolTip("Переместить ниже");
        upBtn->setFixedSize(32,24);
        downBtn->setFixedSize(32,24);
        header->addWidget(upBtn);
        header->addWidget(downBtn);
        l->addLayout(header);

        l->addWidget(contentWidget);
        QHBoxLayout *bottom = new QHBoxLayout();
        bottom->addStretch();
        // Use a ResizeHandle that adjusts only the inner content size (so window doesn't resize)
        ResizeHandle *handle = new ResizeHandle(contentWidget, this);
        bottom->addWidget(handle);
        l->addLayout(bottom);

        initial = contentWidget->sizeHint();
        if (initial.isValid()) this->setMinimumSize(initial);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        connect(upBtn, &QPushButton::clicked, this, [this](){ if (onMove) onMove(-1); });
        connect(downBtn, &QPushButton::clicked, this, [this](){ if (onMove) onMove(+1); });
    }
    template<typename T>
    T* inner() { return qobject_cast<T*>(contentWidget); }
    void setMoveCallback(std::function<void(int)> cb) { onMove = cb; }
    // adjust wrapper minimum height to accommodate inner content height
    void updateForInnerHeight(int newInnerH) {
        int extra = this->height() - contentWidget->height();
        if (extra < 0) extra = 0;
        int newTotal = newInnerH + extra;
        this->setMinimumHeight(newTotal);
        this->resize(this->width(), newTotal);
        this->updateGeometry();
    }
protected:
    void resizeEvent(QResizeEvent *e) override {
        if (!allowShrink) {
            QSize s = e->size();
            if (s.width() < initial.width() || s.height() < initial.height()) {
                this->resize(initial);
                return;
            }
        }
        QFrame::resizeEvent(e);
    }
private:
    QWidget *contentWidget;
    QLabel *titleLabel;
    QPushButton *upBtn;
    QPushButton *downBtn;
    QSize initial;
    bool allowShrink;
    std::function<void(int)> onMove;
};

StatisticsWidget::StatisticsWidget(DataManager *dm, QWidget *parent)
    : QWidget(parent), dm(dm), visitsChartView(nullptr), doctorsChartView(nullptr), topPatientsChartView(nullptr), topDoctorsChartView(nullptr)
{
    buildUI();
    refresh();
}

void StatisticsWidget::buildUI() {
    // Outer layout holds a scroll area so charts can expand vertically
    QVBoxLayout *outer = new QVBoxLayout(this);
    QScrollArea *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QWidget *content = new QWidget();
    QVBoxLayout *main = new QVBoxLayout(content);

    // Header with period label and navigation
    QHBoxLayout *header = new QHBoxLayout();
    prevBtn = new QPushButton();
    prevBtn->setIcon(QIcon(":/images/icon-arrow-left.svg"));
    prevBtn->setIconSize(QSize(16,16));
    prevBtn->setToolTip("Предыдущий период");
    prevBtn->setFixedWidth(36);
    nextBtn = new QPushButton();
    nextBtn->setIcon(QIcon(":/images/icon-arrow-right.svg"));
    nextBtn->setIconSize(QSize(16,16));
    nextBtn->setToolTip("Следующий период");
    nextBtn->setFixedWidth(36);
    periodLabel = new QLabel();
    choosePeriodBtn = new QPushButton("Выбрать период");
    choosePeriodBtn->setIcon(QIcon(":/images/icon-calendar.svg"));
    choosePeriodBtn->setIconSize(QSize(16,16));
    header->addWidget(prevBtn);
    header->addWidget(periodLabel);
    header->addWidget(nextBtn);
    header->addWidget(choosePeriodBtn);
    header->addStretch();
    main->addLayout(header);
    // Initialize chart widgets (or QLabel fallbacks) and stack them vertically
    // chartsLayout holds the resizable chart widgets so we can reorder them
    QWidget *chartsContainer = new QWidget();
    chartsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    chartsLayout = new QVBoxLayout(chartsContainer);
    chartsLayout->setContentsMargins(0,0,0,0);
    chartsLayout->setSpacing(8);
    main->addWidget(chartsContainer);

    {
#ifdef USE_QT_CHARTS
    // create inner chart views and wrap them with resizable wrappers
    // Set sizes to ensure vertical scroll is needed (each chart > 450px height)
    QChartView *innerVisits = new QChartView(new QChart());
    innerVisits->setMinimumHeight(520);
    innerVisits->setMinimumWidth(920);
    visitsChartView = new ResizableWidget(innerVisits, false, "Посещения по неделям");
    static_cast<ResizableWidget*>(visitsChartView)->setMoveCallback([this](int dir){ this->moveChart(static_cast<QWidget*>(visitsChartView), dir); });

    // Note: the detailed weekly aggregated doctors bar chart was removed to reduce redundancy.
    // The visits chart (line series) remains; doctorsChartView is left null and not added to layout.
    doctorsChartView = nullptr;

    QChartView *innerTopPatients = new QChartView(new QChart());
    innerTopPatients->setMinimumHeight(470);
    innerTopPatients->setMinimumWidth(680);
    // pies should only be allowed to grow, not shrink
    topPatientsChartView = new ResizableWidget(innerTopPatients, false, "Топ пациентов");
    static_cast<ResizableWidget*>(topPatientsChartView)->setMoveCallback([this](int dir){ this->moveChart(static_cast<QWidget*>(topPatientsChartView), dir); });

    QChartView *innerTopDoctors = new QChartView(new QChart());
    innerTopDoctors->setMinimumHeight(470);
    innerTopDoctors->setMinimumWidth(680);
    topDoctorsChartView = new ResizableWidget(innerTopDoctors, false, "Топ врачей");
    static_cast<ResizableWidget*>(topDoctorsChartView)->setMoveCallback([this](int dir){ this->moveChart(static_cast<QWidget*>(topDoctorsChartView), dir); });
#else
        QLabel *v1 = new QLabel("Посещения по неделям (Charts не установлен)");
        v1->setAlignment(Qt::AlignCenter);
        v1->setMinimumHeight(140);
        visitsChartView = v1;
        QLabel *v2 = new QLabel("Общее количество приёмов по неделям (Charts не установлен)");
        v2->setAlignment(Qt::AlignCenter);
        v2->setMinimumHeight(140);
        doctorsChartView = v2;
        QLabel *v3 = new QLabel("Топ пациентов (Charts не установлен)");
        v3->setAlignment(Qt::AlignCenter);
        v3->setMinimumHeight(140);
        topPatientsChartView = v3;
        QLabel *v4 = new QLabel("Топ врачей (Charts не установлен)");
        v4->setAlignment(Qt::AlignCenter);
        v4->setMinimumHeight(140);
        topDoctorsChartView = v4;
#endif
        chartsLayout->addWidget(visitsChartView);
        chartsLayout->addWidget(topPatientsChartView);
        chartsLayout->addWidget(topDoctorsChartView);
        chartsLayout->addStretch();
    }

    // Footer with refresh only (suggestions removed)
    QHBoxLayout *footer = new QHBoxLayout();
    QPushButton *refreshBtn = new QPushButton("Обновить данные");
    refreshBtn->setIcon(QIcon(":/images/icon-refresh.svg"));
    refreshBtn->setIconSize(QSize(16,16));
    footer->addWidget(refreshBtn);
    footer->addStretch();
    main->addLayout(footer);

    connect(refreshBtn, &QPushButton::clicked, this, &StatisticsWidget::refresh);
    connect(prevBtn, &QPushButton::clicked, this, [this]() { this->customPeriod = false; this->endOffsetWeeks += 1; this->refresh(); });
    connect(nextBtn, &QPushButton::clicked, this, [this]() { this->customPeriod = false; if (this->endOffsetWeeks > 0) this->endOffsetWeeks -= 1; this->refresh(); });

    connect(choosePeriodBtn, &QPushButton::clicked, this, [this](){
        QDialog dlg(this);
        dlg.setWindowTitle("Выбрать период");
        QVBoxLayout *l = new QVBoxLayout(&dlg);
        QHBoxLayout *h = new QHBoxLayout();
        QDateEdit *start = new QDateEdit();
        start->setCalendarPopup(true);
        QDateEdit *end = new QDateEdit();
        end->setCalendarPopup(true);
        QDate today = QDate::currentDate();
        // sensible defaults: default to current month
        start->setDate(this->customPeriod ? this->periodStart : QDate(today.year(), today.month(), 1));
        end->setDate(this->customPeriod ? this->periodEnd : today);
        h->addWidget(new QLabel("Начало:"));
        h->addWidget(start);
        h->addWidget(new QLabel("Конец:"));
        h->addWidget(end);
        l->addLayout(h);
        QHBoxLayout *buttons = new QHBoxLayout();
        buttons->addStretch();
        QPushButton *ok = new QPushButton("OK");
        QPushButton *cancel = new QPushButton("Отмена");
        buttons->addWidget(ok);
        buttons->addWidget(cancel);
        l->addLayout(buttons);
        connect(ok, &QPushButton::clicked, &dlg, &QDialog::accept);
        connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
        if (dlg.exec() == QDialog::Accepted) {
            this->periodStart = start->date();
            this->periodEnd = end->date();
            if (!this->periodStart.isValid() || !this->periodEnd.isValid() || this->periodStart > this->periodEnd) {
                // invalid selection - ignore
                return;
            }
            this->customPeriod = true;
            this->refresh();
        }
    });

    // put the content into the scroll area and add to the outer layout
    scroll->setWidget(content);
    outer->addWidget(scroll);
}

void StatisticsWidget::refresh() {
    buildWeeklyCharts();
    buildTopLists();
}

void StatisticsWidget::moveChart(QWidget *w, int direction) {
    if (!w || !chartsLayout) return;
    int count = chartsLayout->count();
    // find current index
    int idx = -1;
    for (int i = 0; i < count; ++i) {
        QLayoutItem *it = chartsLayout->itemAt(i);
        if (!it) continue;
        if (it->widget() == w) { idx = i; break; }
    }
    if (idx == -1) return;
    int newIdx = idx + direction;
    // ensure within 0..count-1 (but ignore trailing stretch)
    int maxIdx = qMax(0, count - 2); // last real widget before stretch
    if (newIdx < 0) newIdx = 0;
    if (newIdx > maxIdx) newIdx = maxIdx;
    if (newIdx == idx) return;
    chartsLayout->removeWidget(w);
    // insert at new index
    chartsLayout->insertWidget(newIdx, w);
}

static int weekKey(const QDateTime &dt) {
    // Represent weeks as year*100 + weekNumber
    int year = 0;
    int week = dt.date().weekNumber(&year);
    return year * 100 + week;
}

void StatisticsWidget::buildWeeklyCharts() {
    QList<Appointment> appts = dm->getAllAppointments();

    // Determine date range: either a custom period selected via calendar, or default last 12 weeks
    QDate startDate;
    QDate endDate;
    if (this->customPeriod && this->periodStart.isValid() && this->periodEnd.isValid()) {
        startDate = this->periodStart;
        endDate = this->periodEnd;
    } else {
        QDate today = QDate::currentDate();
        // default period: current month (start = 1st of month), end = today shifted by endOffsetWeeks
        endDate = today.addDays(-7 * endOffsetWeeks);
        startDate = QDate(endDate.year(), endDate.month(), 1);
    }

    // Build list of week keys covering the selected period (one key per 7-day step)
    QList<int> weekKeys;
    int days = startDate.daysTo(endDate);
    int weeks = qMax(1, days / 7 + 1);
    for (int i = 0; i < weeks; ++i) {
        QDate d = startDate.addDays(7 * i);
        int year = 0;
        int week = d.weekNumber(&year);
        weekKeys.append(year * 100 + week);
    }

    // Update period label
    periodLabel->setText(QString("Период: %1 — %2").arg(startDate.toString("yyyy-MM-dd"), endDate.toString("yyyy-MM-dd")));

    QMap<int,int> patientsPerWeek;
    QMap<int,int> doctorsPerWeek;
    for (const Appointment &a : appts) {
        if (!a.date.isValid()) continue;
        int k = weekKey(a.date);
        // only include weeks in the range
        if (!weekKeys.contains(k)) continue;
        patientsPerWeek[k] += 1; // counts appointments (proxy for patient visits)
        doctorsPerWeek[k] += 1;
    }

    // Line/Bar series for charts (or text summaries when Qt Charts not available)
    QStringList categories;

    // use formatShortPerson helper

#ifdef USE_QT_CHARTS
    QLineSeries *patientsSeries = new QLineSeries();
    patientsSeries->setName("Посещения (пациенты)");
    patientsSeries->setColor(QColor(medicalPrimaryColor()));

    QLineSeries *doctorsSeries = new QLineSeries();
    doctorsSeries->setName("Посещения (врачи)");
    doctorsSeries->setColor(QColor(medicalAccentColor()));

    // If a custom period is selected, show results split by day across the period
    if (this->customPeriod) {
        int totalDays = startDate.daysTo(endDate) + 1;
        for (int i = 0; i < totalDays; ++i) {
            QDate d = startDate.addDays(i);
            int countP = 0, countD = 0;
            for (const Appointment &a : appts) {
                if (!a.date.isValid()) continue;
                if (a.date.date() == d) {
                    countP += 1;
                    countD += 1;
                }
            }
            patientsSeries->append(i, countP);
            doctorsSeries->append(i, countD);
            categories << d.toString("dd.MM");
        }
    } else {
        // populate series and categories by week (default behavior)
        for (int i = 0; i < weekKeys.size(); ++i) {
            int k = weekKeys[i];
            int valP = patientsPerWeek.value(k, 0);
            int valD = doctorsPerWeek.value(k, 0);
            patientsSeries->append(i, valP);
            doctorsSeries->append(i, valD);
            // label by week start date
            int wk = k % 100;
            int yr = k / 100;
            QDate d = QDate::fromString(QString("%1-W%2-1").arg(yr).arg(wk, 2, 10, QChar('0')), "yyyy-'W'ww-'1'");
            QString label = d.isValid() ? d.toString("dd.MM") : QString::number(wk);
            categories << label;
        }
    }

    QChart *chart = new QChart();
    chart->addSeries(patientsSeries);
    chart->addSeries(doctorsSeries);
    chart->legend()->setVisible(true);
    if (this->customPeriod) chart->setTitle(QString("Посещения за период %1 — %2").arg(startDate.toString("dd.MM.yyyy"), endDate.toString("dd.MM.yyyy")));
    else chart->setTitle("Посещения по неделям (последние 12 недель)");

    QValueAxis *axisY = new QValueAxis();
    axisY->setLabelFormat("%d");
    axisY->setTitleText("Кол-во приёмов");
    chart->addAxis(axisY, Qt::AlignLeft);
    patientsSeries->attachAxis(axisY);
    doctorsSeries->attachAxis(axisY);

    QCategoryAxis *axisX = new QCategoryAxis();
    // restore: append every category so each point on chart 1 has a label
    for (int i = 0; i < categories.size(); ++i) axisX->append(categories[i], i);
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axisX->setTitleText(this->customPeriod ? "Дни (левее — ранее)" : "Недели (левая — более ранняя)");
    chart->addAxis(axisX, Qt::AlignBottom);
    patientsSeries->attachAxis(axisX);
    doctorsSeries->attachAxis(axisX);

    {
        ResizableWidget *wrap = static_cast<ResizableWidget*>(visitsChartView);
        QChartView *v = wrap ? wrap->inner<QChartView>() : nullptr;
            if (v) {
                v->setChart(chart);
                v->setRenderHint(QPainter::Antialiasing);
                // enable hover tooltips for line points
                patientsSeries->setPointsVisible(true);
                connect(patientsSeries, &QXYSeries::hovered, this, [=](const QPointF &point, bool state){
                    if (!state) return;
                    // only show tooltip for exact data points (integer X)
                    double x = point.x();
                    int idx = qRound(x);
                    if (qAbs(x - idx) > 1e-6) return; // intermediate point, ignore
                    QString label = categories.value(idx, "");
                    QString txt = QString("%1: %2 приёмов").arg(label).arg((int)point.y());
                    QToolTip::showText(QCursor::pos(), txt);
                });
                doctorsSeries->setPointsVisible(true);
                connect(doctorsSeries, &QXYSeries::hovered, this, [=](const QPointF &point, bool state){
                    if (!state) return;
                    double x = point.x();
                    int idx = qRound(x);
                    if (qAbs(x - idx) > 1e-6) return;
                    QString label = categories.value(idx, "");
                    QString txt = QString("%1: %2 приёмов").arg(label).arg((int)point.y());
                    QToolTip::showText(QCursor::pos(), txt);
                });
            }
    }

    // Doctor-specific aggregated weekly chart (bar chart per week showing total visits)
    QBarSeries *barSeries = new QBarSeries();
    QBarSet *set = new QBarSet("Приёмы");
    set->setColor(QColor(medicalAccentColor()));
    QChart *chart2 = new QChart();
    // If custom period selected, aggregate into a single bar for the period
    if (this->customPeriod) {
        int total = 0;
        for (const Appointment &a : appts) {
            if (!a.date.isValid()) continue;
            if (a.date.date() < startDate || a.date.date() > endDate) continue;
            total += 1;
        }
        *set << total;
        barSeries->append(set);
        chart2->addSeries(barSeries);
        chart2->setTitle(QString("Общее количество приёмов: %1 — %2").arg(startDate.toString("dd.MM.yyyy"), endDate.toString("dd.MM.yyyy")));
        QBarCategoryAxis *axisX2 = new QBarCategoryAxis();
        axisX2->append(QStringList{QString("%1 — %2").arg(startDate.toString("dd.MM"), endDate.toString("dd.MM"))});
        chart2->addAxis(axisX2, Qt::AlignBottom);
        barSeries->attachAxis(axisX2);
        QValueAxis *axisY2 = new QValueAxis();
        axisY2->setTitleText("Кол-во приёмов");
        chart2->addAxis(axisY2, Qt::AlignLeft);
        barSeries->attachAxis(axisY2);
    } else {
        for (int i = 0; i < weekKeys.size(); ++i) {
            int v = patientsPerWeek.value(weekKeys[i], 0);
            *set << v;
        }
        barSeries->append(set);
        chart2->addSeries(barSeries);
        chart2->setTitle("Общее количество приёмов по неделям");
        QBarCategoryAxis *axisX2 = new QBarCategoryAxis();
        QStringList cat2;
        for (int i = 0; i < categories.size(); ++i) cat2 << categories[i];
        axisX2->append(cat2);
        chart2->addAxis(axisX2, Qt::AlignBottom);
        barSeries->attachAxis(axisX2);
        QValueAxis *axisY2 = new QValueAxis();
        axisY2->setTitleText("Кол-во приёмов");
        chart2->addAxis(axisY2, Qt::AlignLeft);
        barSeries->attachAxis(axisY2);
    }
    {
        ResizableWidget *wrap = static_cast<ResizableWidget*>(doctorsChartView);
        QChartView *v = wrap ? wrap->inner<QChartView>() : nullptr;
        if (v) {
                v->setChart(chart2);
                v->setRenderHint(QPainter::Antialiasing);
                // enable hover on bars (use QBarSet::hovered)
                connect(set, &QBarSet::hovered, this, [=](int index, bool status){
                    if (!status) return;
                    QString label = index >=0 && index < categories.size() ? categories[index] : QString();
                    QString periodType = this->customPeriod ? "Дата: " : "Неделя: ";
                    QString txt = QString("%1%2\nВсего приёмов: %3").arg(periodType).arg(label).arg(set->at(index));
                    QToolTip::showText(QCursor::pos(), txt);
                });
            }
    }
#else
    // Charts not available: build categories and show simple summaries
    for (int i = 0; i < weekKeys.size(); ++i) {
        int k = weekKeys[i];
        // label by week start date
        int wk = k % 100;
        int yr = k / 100;
        QDate d = QDate::fromString(QString("%1-W%2-1").arg(yr).arg(wk, 2, 10, QChar('0')), "yyyy-'W'ww-'1'");
        QString label = d.isValid() ? d.toString("dd.MM") : QString::number(wk);
        categories << label;
    }
    QString visitsSummary = "Посещения по неделям:\n";
    for (int i = 0; i < weekKeys.size(); ++i) {
        visitsSummary += QString("%1: %2\n").arg(categories.value(i)).arg(patientsPerWeek.value(weekKeys[i],0));
    }
    static_cast<QLabel*>(visitsChartView)->setText(visitsSummary);
    QString doctorsSummary = "Приёмы по неделям:\n";
    for (int i = 0; i < weekKeys.size(); ++i) {
        doctorsSummary += QString("%1: %2\n").arg(categories.value(i)).arg(doctorsPerWeek.value(weekKeys[i],0));
    }
    static_cast<QLabel*>(doctorsChartView)->setText(doctorsSummary);
#endif
}

void StatisticsWidget::buildTopLists() {
    QList<Appointment> appts = dm->getAllAppointments();
    QMap<int,int> countByPatient;
    QMap<int,int> countByDoctor;
    for (const Appointment &a : appts) {
        countByPatient[a.id_patient] += 1;
        countByDoctor[a.id_doctor] += 1;
    }
    // Top patients
    QList<QPair<int,int>> patientList;
    for (auto it = countByPatient.begin(); it != countByPatient.end(); ++it) patientList.append({it.key(), it.value()});
    std::stable_sort(patientList.begin(), patientList.end(), [](const QPair<int,int>& a, const QPair<int,int>& b){ return a.second > b.second; });

    #ifdef USE_QT_CHARTS
        QPieSeries *ps = new QPieSeries();
        int added = 0;
        for (const auto &p : patientList) {
            if (added >= 5) break; // show only top 5
            Patient pat = dm->getPatientById(p.first);
            QString label = pat.fullName().isEmpty() ? QString("Пациент %1").arg(p.first) : formatShortPerson(pat.lname, pat.fname, pat.tname, pat.id_patient);
            QPieSlice *slice = ps->append(label, p.second);
            ++added;
        }
                    QChart *chart = new QChart();
                    chart->addSeries(ps);
                chart->setTitle("Топ пациентов по количеству приёмов");
                {
                    ResizableWidget *wrap = static_cast<ResizableWidget*>(topPatientsChartView);
                    QChartView *v = wrap ? wrap->inner<QChartView>() : nullptr;
                    if (v) {
                        v->setChart(chart);
                        v->setRenderHint(QPainter::Antialiasing);
                        // enable hover tooltips + подсветка как у диаграммы врачей
                        for (QPieSlice *s : ps->slices()) {
                            s->setLabelVisible(false);
                            connect(s, &QPieSlice::hovered, this, [=](bool state){
                                s->setLabelVisible(state);
                                s->setExploded(state);
                                if (!state) { QToolTip::hideText(); return; }
                                QString txt = QString("%1 — %2 приёмов").arg(s->label()).arg((int)s->value());
                                QToolTip::showText(QCursor::pos(), txt);
                            });
                        }
                    }
                }

    // Top doctors - show as a pie chart (hover shows slice label + details)
    QList<QPair<int,int>> doctorList;
    for (auto it = countByDoctor.begin(); it != countByDoctor.end(); ++it) doctorList.append({it.key(), it.value()});
    std::stable_sort(doctorList.begin(), doctorList.end(), [](const QPair<int,int>& a, const QPair<int,int>& b){ return a.second > b.second; });
    QPieSeries *dps = new QPieSeries();
    int dadded = 0;
    for (const auto &d : doctorList) {
        if (dadded >= 8) break; // show top 8 as pie slices
        Doctor doc = dm->getDoctorById(d.first);
        QString label = doc.fullName().isEmpty() ? QString("Врач %1").arg(d.first) : formatShortPerson(doc.lname, doc.fname, doc.tname, doc.id_doctor);
        QPieSlice *slice = dps->append(label, d.second);
        slice->setLabelVisible(false);
        ++dadded;
    }
    QChart *chartD = new QChart();
    chartD->addSeries(dps);
    chartD->setTitle("Топ врачей по количеству приёмов");
    {
        ResizableWidget *wrap = static_cast<ResizableWidget*>(topDoctorsChartView);
        QChartView *v = wrap ? wrap->inner<QChartView>() : nullptr;
        if (v) {
            v->setChart(chartD);
            v->setRenderHint(QPainter::Antialiasing);
            // show tooltip and toggle slice label on hover
            for (QPieSlice *s : dps->slices()) {
                connect(s, &QPieSlice::hovered, this, [=](bool state){
                    if (!state) {
                        s->setLabelVisible(false);
                        return;
                    }
                    s->setLabelVisible(true);
                    int val = (int)s->value();
                    QString label = s->label();
                    // attempt to find doctor id from label by searching doctorList: fallback info will be label only
                    QString extra = QString("Приёмов: %1").arg(val);
                    // try to find specialization for nicer tooltip (best-effort)
                    for (const auto &dp : doctorList) {
                        Doctor doc2 = dm->getDoctorById(dp.first);
                        QString shortLabel = doc2.fullName().isEmpty() ? QString("Врач %1").arg(doc2.id_doctor) : formatShortPerson(doc2.lname, doc2.fname, doc2.tname, doc2.id_doctor);
                        if (shortLabel == label) {
                            Specialization spec = dm->getSpecializationById(doc2.id_spec);
                            extra = QString("%1\nСпециальность: %2\nПриёмов: %3").arg(label).arg(spec.name).arg(val);
                            break;
                        }
                    }
                    QToolTip::showText(QCursor::pos(), extra);
                });
            }
        }
    }
#else
    // Charts not available: show text summaries with limits
    QString topPatientsText = "Топ пациентов:\n";
    int maxDisplay = 10;
    int count = 0;
    for (const auto &p : patientList) {
        if (count++ >= maxDisplay) {
            topPatientsText += "... (и еще " + QString::number(patientList.size() - maxDisplay) + ")\n";
            break;
        }
        Patient pat = dm->getPatientById(p.first);
        QString label = pat.fullName().isEmpty() ? QString("Пациент %1").arg(p.first) : formatShortPerson(pat.lname, pat.fname, pat.tname, pat.id_patient);
        topPatientsText += QString("%1 — %2\n").arg(label).arg(p.second);
    }
    static_cast<QLabel*>(topPatientsChartView)->setText(topPatientsText);

    QString topDoctorsText = "Топ врачей:\n";
    count = 0;
    QList<QPair<int,int>> doctorList;
    for (auto it = countByDoctor.begin(); it != countByDoctor.end(); ++it) doctorList.append({it.key(), it.value()});
    std::stable_sort(doctorList.begin(), doctorList.end(), [](const QPair<int,int>& a, const QPair<int,int>& b){ return a.second > b.second; });
    for (const auto &d : doctorList) {
        if (count++ >= maxDisplay) {
            topDoctorsText += "... (и еще " + QString::number(doctorList.size() - maxDisplay) + ")\n";
            break;
        }
        Doctor doc = dm->getDoctorById(d.first);
        QString label = doc.fullName().isEmpty() ? QString("Врач %1").arg(d.first) : formatShortPerson(doc.lname, doc.fname, doc.tname, doc.id_doctor);
        topDoctorsText += QString("%1 — %2\n").arg(label).arg(d.second);
    }
    static_cast<QLabel*>(topDoctorsChartView)->setText(topDoctorsText);
#endif
}
