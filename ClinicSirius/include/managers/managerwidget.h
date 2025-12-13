#ifndef MANAGERWIDGET_H
#define MANAGERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "common/datamanager.h"
#include "common/models.h"

class ManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit ManagerWidget(QWidget* parent = nullptr);
    void setUser(const LoginUser& user);

private slots:
    void refreshAppointments();
    void onAddAppointment();
    void onCancelAppointment();
    void onRescheduleAppointment();
    // Clinic management actions
    void onViewClinicSchedules();
    void onManagePatients();
    void onBulkOperations();

private:
    DataManager m_dataManager;
    LoginUser m_user;

    QListWidget* m_appointmentsList;
    QPushButton* m_addBtn;
    QPushButton* m_cancelBtn;
    QPushButton* m_rescheduleBtn;
    // Clinic management buttons
    QPushButton* m_viewSchedulesBtn;
    QPushButton* m_managePatientsBtn;
    QPushButton* m_bulkOpsBtn;
};

#endif // MANAGERWIDGET_H
