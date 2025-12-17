#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDateEdit>
#include "models.h"
#include "datamanager.h"
#include "scrollabletab.h"
#include "statsbadge.h"

class ProfileWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget *parent = nullptr);
    void setUser(const LoginUser &user);
    void openSettingsTab();

signals:
    void requestLogout();
    void requestAccountDeletion();

private slots:
    void onRemoveFamilyMember();
    void onEditFamilyMember(QListWidgetItem *item);
    void onGenerateInvitationCode();
    void onUseInvitationCode();
    void onSaveProfile();
    void onDeleteAccount();
    void onRefreshData();

private:
    void buildUI();
    void loadProfile();
    void loadFamily();
    void buildProfileTab();
    void buildSettingsTab();
    void connectSignals();

    LoginUser currentUser;
    DataManager dataManager;

    QTabWidget *tabs;

    // Profile tab
    QLabel *nameValue;
    QLabel *emailValue;
    QLabel *phoneValue;
    QLabel *birthValue;
    QLabel *snilsValue;
    QLabel *omsValue;
    QListWidget *familyList;
    QLabel *addFamilyStatus;
    QPushButton *removeFamilyButton;
    QPushButton *generateCodeButton;
    QPushButton *useCodeButton;
    QPushButton *refreshDataButton;
    QLineEdit *invitationCodeDisplay;
    QLabel *codeStatusLabel;

    // Modern UI components
    StatsBadge *familyCountBadge;
    QLineEdit *familySearchBox;

    // Settings tab
    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *middleNameEdit;
    QDateEdit *birthEdit;
    QLineEdit *emailEdit;
    QLineEdit *phoneEdit;
    QLineEdit *snilsEdit;
    QLineEdit *omsEdit;
    QLabel *saveStatusLabel;
    QPushButton *saveButton;
    QPushButton *deleteButton;
};

#endif // PROFILEWIDGET_H
