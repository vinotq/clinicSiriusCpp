#ifndef MANAGERSCHEDULEVIEWER_H
#define MANAGERSCHEDULEVIEWER_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QDate>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include "common/datamanager.h"

class ManagerScheduleViewer : public QDialog {
    Q_OBJECT
public:
    explicit ManagerScheduleViewer(QWidget *parent = nullptr);

private slots:
    void onDoctorFilterChanged(const QString &text);
    void onDoctorSelected(int index);
    void onPrevWeek();
    void onNextWeek();
    void onToday();

private:
    void buildUI();
    void loadDoctors();
    void loadScheduleForDoctor(int doctorId);

    DataManager m_dataManager;
    QLineEdit *m_filterEdit;
    QComboBox *m_doctorCombo;
    QTableWidget *m_scheduleTable;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_todayBtn;
    QLabel *m_weekLabel;
    QDate m_startDate;
    QList<Doctor> m_allDoctors;
    int m_currentDoctorId = -1;
};

#endif // MANAGERSCHEDULEVIEWER_H
