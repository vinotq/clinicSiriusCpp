#ifndef MANAGERPROFILEWIDGET_H
#define MANAGERPROFILEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include "common/models.h"
#include "common/datamanager.h"

class ManagerProfileWidget : public QWidget {
    Q_OBJECT
public:
    explicit ManagerProfileWidget(QWidget *parent = nullptr);
    void setUser(const LoginUser &user);

signals:
    void requestLogout();

private slots:
    void onRegisterPatient();
    void onAttachToFamily();
    void onViewClinicSchedule();

private:
    void buildUI();
    void loadManagerInfo();

    LoginUser m_user;
    DataManager m_dataManager;

    QLabel *nameLabel;
    QLabel *emailLabel;
    QPushButton *registerPatientBtn;
    QPushButton *attachFamilyBtn;
    QPushButton *viewScheduleBtn;
};

#endif // MANAGERPROFILEWIDGET_H
