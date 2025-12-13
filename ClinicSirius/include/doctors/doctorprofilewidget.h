#ifndef DOCTORPROFILEWIDGET_H
#define DOCTORPROFILEWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDateEdit>
#include "models.h"
#include "datamanager.h"

class DoctorProfileWidget : public QWidget {
    Q_OBJECT

public:
    explicit DoctorProfileWidget(QWidget *parent = nullptr);
    void setUser(const LoginUser &user);
    void openSettingsTab();

signals:
    void requestLogout();
    void requestAccountDeletion();

private slots:
    void onSaveProfile();
    void onDeleteAccount();

private:
    void buildUI();
    void loadProfile();

    LoginUser currentUser;
    DataManager dataManager;

    QTabWidget *tabs;

    // Profile tab
    QLabel *nameValue;
    QLabel *emailValue;
    QLabel *phoneValue;
    QLabel *birthValue;
    QLabel *specValue;

    // Settings tab
    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *middleNameEdit;
    QDateEdit *birthEdit;
    QLineEdit *emailEdit;
    QLineEdit *phoneEdit;
    QLabel *saveStatusLabel;
    QPushButton *saveButton;
    QPushButton *deleteButton;
};

#endif // DOCTORPROFILEWIDGET_H
