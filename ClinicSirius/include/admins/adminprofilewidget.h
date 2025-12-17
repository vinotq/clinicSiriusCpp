#ifndef ADMINPROFILEWIDGET_H
#define ADMINPROFILEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "common/models.h"
#include "common/datamanager.h"

class AdminProfileWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdminProfileWidget(QWidget *parent = nullptr);
    void setUser(const LoginUser &user);

signals:
    void requestLogout();

private slots:
    void onSaveSettings();

private:
    void buildUI();
    void loadAdminInfo();

    LoginUser m_user;
    DataManager m_dataManager;
    
    QLabel* m_userIdLabel;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_logoutBtn;
};

#endif // ADMINPROFILEWIDGET_H

