#ifndef DOCTORWIDGET_H
#define DOCTORWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QDate>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
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
    void onViewSchedule();
    void onAddSlot();
    void onBookAppointment();
    void onProfileClicked();
    void onSettingsClicked();

    void onCellClicked(int row, int column);
    void onCellDoubleClicked(int row, int column);
    void loadSchedule();
    void onBackFromSchedule();
    void onPrevWeek();
    void onNextWeek();
    void onToday();

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

    QLabel *mainTitleLabel;
    QPushButton *viewScheduleButton;
    QPushButton *addSlotButton;
    QPushButton *bookAppointmentButton;

    QSpinBox *timeSlotDurationSpinBox = nullptr;
    int selectedIntervalMinutes = 20;

    QLabel *scheduleTitleLabel;
    QTableWidget *scheduleTable;
    QPushButton *bookFromScheduleButton;
    QPushButton *backButton;
    QPushButton *deleteSlotButton;
    QPushButton *addSlotInScheduleButton;
    QPushButton *prevWeekButton;
    QPushButton *nextWeekButton;
    QPushButton *todayButton;
    QLabel *weekLabel;
    QDate scheduleStartDate;
};

#endif
