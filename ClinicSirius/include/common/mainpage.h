#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QMenu>
#include "models.h"

class ProfileWidget;
class DoctorWidget;
class DoctorProfileWidget;
class AppointmentBookingWidget;
class PatientHistoryWidget;

class ServiceCard : public QWidget {
    Q_OBJECT

public:
    explicit ServiceCard(const QString& title, const QString& description, 
                        const QString& iconPath, QWidget *parent = nullptr);

private:
    void setupUI(const QString& title, const QString& description, const QString& iconPath);
    void applyStyles();
};

class MainPage : public QWidget {
    Q_OBJECT

public:
    explicit MainPage(QWidget *parent = nullptr);
    ~MainPage();
    void setCurrentUser(const LoginUser &user);

signals:
    void logoutRequested();
    void logout();
    void navigateToBooking();
    void navigateToDoctors();

private:
    void setupUI();
    void applyStyles();
    void buildLanding();
    void buildDoctorLanding();
    void buildHeader(QHBoxLayout *headerLayout);
    void updateUserChip();
    void showProfile(bool openSettingsTab = false);
    void showDoctorProfile(bool openSettingsTab = false);
    void showHome();
    void showBooking();

    QPushButton *logoButton;
    QToolButton *userMenuButton;
    QMenu *userMenu;
    QAction *profileAction;
    QAction *settingsAction;
    QAction *logoutAction;
    QLabel *welcomeLabel;
    QLabel *descriptionLabel;
    QVBoxLayout *mainLayout;
    QStackedWidget *contentStack;
    QWidget *landingPage;
    QWidget *doctorLandingPage;
    ProfileWidget *profileWidget;
    DoctorProfileWidget *doctorProfileWidget;
    DoctorWidget *doctorWidget;
    AppointmentBookingWidget *appointmentBookingWidget;
    PatientHistoryWidget *patientHistoryWidget;
    class ManagerWidget *managerWidget;
    class ManagerProfileWidget *managerProfileWidget;
    LoginUser currentUser;
};

#endif // MAINPAGE_H
