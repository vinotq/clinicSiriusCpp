#ifndef MANAGERPROFILEWIDGET_H
#define MANAGERPROFILEWIDGET_H

#include <QWidget>
#include <QLineEdit>
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
    void onSaveSettings();

private:
    void buildUI();
    void loadManagerInfo();

    LoginUser m_user;
    DataManager m_dataManager;
    
    QLabel* m_userIdLabel;
    QLineEdit* m_firstNameEdit;
    QLineEdit* m_lastNameEdit;
    QLineEdit* m_patronymicEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_phoneEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_logoutBtn;
};

#endif // MANAGERPROFILEWIDGET_H
