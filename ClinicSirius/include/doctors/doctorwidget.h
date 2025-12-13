#ifndef DOCTORWIDGET_H
#define DOCTORWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QDate>
#include <QVBoxLayout>
#include <QLabel>
#include "models.h"
#include "datamanager.h"

class DoctorVisitDialog;

class DoctorWidget : public QWidget {
    Q_OBJECT
public:
    explicit DoctorWidget(QWidget *parent = nullptr);
    void setUser(const LoginUser &user);

signals:
    void requestLogout();
    void requestPageChange(int pageIndex);

private slots:
    // Main page
    void onViewSchedule();
    void onAddSlot();
    void onBookAppointment();
    void onProfileClicked();
    void onSettingsClicked();

    // Schedule page
    void onCellClicked(int row, int column);
    void loadSchedule();
    void onBackFromSchedule();
    void onPrevWeek();
    void onNextWeek();
    void onToday();

    // After visit completion
    void onVisitCompleted();

private:
    void buildMainPage();
    void buildSchedulePage();
    void buildUI();
    void refreshHeader();
    void populateScheduleTable();

    LoginUser currentUser;
    DataManager dataManager;

    QStackedWidget *stackedWidget;
    int mainPageIndex;
    int schedulePageIndex;

    // Main page widgets
    QLabel *mainTitleLabel;
    QPushButton *viewScheduleButton;
    QPushButton *addSlotButton;
    QPushButton *bookAppointmentButton;

    // Schedule page widgets
    QLabel *scheduleTitleLabel;
    QTableWidget *scheduleTable;
    QPushButton *bookFromScheduleButton;
    QPushButton *backButton;
    QPushButton *deleteSlotButton;
    QPushButton *addSlotInScheduleButton; // новая кнопка в расписании
    QPushButton *prevWeekButton;
    QPushButton *nextWeekButton;
    QPushButton *todayButton;
    QLabel *weekLabel;
    QDate scheduleStartDate;
};

#endif // DOCTORWIDGET_H
