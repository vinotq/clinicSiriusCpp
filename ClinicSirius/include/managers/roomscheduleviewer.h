#ifndef ROOMSCHEDULEVIEWER_H
#define ROOMSCHEDULEVIEWER_H

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

class RoomScheduleViewer : public QWidget {
    Q_OBJECT
public:
    explicit RoomScheduleViewer(QWidget *parent = nullptr);

private slots:
    void onRoomFilterChanged(const QString &text);
    void onPrevWeek();
    void onNextWeek();
    void onToday();
    void onIntervalChanged(int value);
    void onTableContextMenu(const QPoint &pos);

private:
    void buildUI();
    void loadRooms();
    void loadScheduleForRoom(int roomId);
    void applyRoomFilter(const QString &text);
    QDate getMondayOfWeek(const QDate &date) const;

    DataManager m_dataManager;
    QLineEdit *m_filterEdit;
    QCompleter *m_roomCompleter;
    QTableWidget *m_scheduleTable;
    QPushButton *m_prevBtn;
    QPushButton *m_nextBtn;
    QPushButton *m_todayBtn;
    QSpinBox *m_intervalSpin;
    QLabel *m_weekLabel;
    QDate m_startDate;
    QList<Room> m_allRooms;
    int m_currentRoomId = -1;
    int m_timeIntervalMinutes = 20;
};

#endif // ROOMSCHEDULEVIEWER_H
