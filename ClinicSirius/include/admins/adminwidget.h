#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include "models.h"

class DataManager;

class AdminWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdminWidget(QWidget *parent = nullptr);
    void setUser(const LoginUser &user);

private slots:
    // Doctors
    void loadDoctors();
    void onAddDoctor();
    void onEditDoctor();
    void onDeleteDoctor();
    void onManageDoctorSchedule();

    // Patients
    void loadPatients();
    void onAddPatient();
    void onEditPatient();
    void onDeletePatient();
    void onViewPatientAppointments();

    // Managers
    void loadManagers();
    void onAddManager();
    void onEditManager();
    void onDeleteManager();

    // Search/filter
    void onDoctorsFilterChanged(const QString &text);
    void onPatientsFilterChanged(const QString &text);
    void onManagersFilterChanged(const QString &text);

private:
    void buildUI();

    QTabWidget *tabs;

    // Doctors tab
    QWidget *doctorsTab;
    QLineEdit *doctorsSearchEdit;
    QTableWidget *doctorsTable;
    QPushButton *addDoctorBtn;
    QPushButton *editDoctorBtn;
    QPushButton *deleteDoctorBtn;
    QPushButton *manageScheduleBtn;

    // Patients tab
    QWidget *patientsTab;
    QLineEdit *patientsSearchEdit;
    QTableWidget *patientsTable;
    QPushButton *addPatientBtn;
    QPushButton *editPatientBtn;
    QPushButton *deletePatientBtn;
    QPushButton *viewAppointmentsBtn;

    // Managers tab
    QWidget *managersTab;
    QLineEdit *managersSearchEdit;
    QTableWidget *managersTable;
    QPushButton *addManagerBtn;
    QPushButton *editManagerBtn;
    QPushButton *deleteManagerBtn;

    // Statistics tab
    QWidget *statisticsTab;
    class StatisticsWidget *statisticsWidget;

    LoginUser currentUser;
    DataManager *dataManager;
    
    // Store full data for filtering
    QList<Doctor> allDoctors;
    QList<Patient> allPatients;
    QList<Manager> allManagers;
};

#endif // ADMINWIDGET_H
