#ifndef MANAGERSCHEDULEVIEWER_H
#define MANAGERSCHEDULEVIEWER_H

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QCompleter>
#include <QDate>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QList>
#include "common/datamanager.h"

class ManagerScheduleViewer : public QWidget {
    Q_OBJECT
public:
    explicit ManagerScheduleViewer(QWidget *parent = nullptr);
    // New constructor that uses existing DataManager instance
    explicit ManagerScheduleViewer(DataManager *dm, QWidget *parent = nullptr);
    // Set the currently selected doctor and refresh the view
    void setCurrentDoctor(int doctorId);

private slots:
    void onDoctorFilterChanged(const QString &text);
    void onPrevWeek();
    void onNextWeek();
    void onToday();
    void onIntervalChanged(int value);
    void onTableContextMenu(const QPoint &pos);

private:
    void buildUI();
    void loadDoctors();
    void loadScheduleForDoctor(int doctorId);
    void applyDoctorFilter(const QString &text);
    QDate getMondayOfWeek(const QDate &date) const;

    // Use pointer to shared DataManager so viewers share the same data source
    DataManager *m_dataManager = nullptr;
    QLineEdit *m_filterEdit;
    QCompleter *m_doctorCompleter;
    QTableWidget *m_scheduleTable;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_todayBtn;
    QSpinBox *m_intervalSpin;
    QLabel *m_weekLabel;
    QDate m_startDate;
    QList<Doctor> m_allDoctors;
    int m_currentDoctorId = -1;
    int m_timeIntervalMinutes = 20;
};

#endif // MANAGERSCHEDULEVIEWER_H
