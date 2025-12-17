#ifndef FAMILYVIEWERWIDGET_H
#define FAMILYVIEWERWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "common/datamanager.h"

class FamilyViewerWidget : public QWidget {
    Q_OBJECT

public:
    explicit FamilyViewerWidget(QWidget *parent = nullptr);
    void setUser(const LoginUser &user);
    void setSelectedPatient(int patientId);

private slots:
    void onPatientSelected(int index);
    void onAddMember();
    void onEditMember();
    void onRemoveMember();
    void onMemberDoubleClicked(QListWidgetItem *item);
    void onSearchPatient(const QString &text);
    void loadFamilyMembers();

private:
    void buildUI();
    void refreshFamilyList();
    void populatePatientSelector();
    void updatePatientSelector(int selectedPatientId = -1);

    DataManager m_dataManager;
    LoginUser m_currentUser;
    int m_selectedPatientId = -1;

    // Patient selector
    QLineEdit *m_patientSearchEdit = nullptr;
    QComboBox *m_patientComboBox = nullptr;
    QLabel *m_patientInfoLabel = nullptr;

    // Family list
    QListWidget *m_familyList = nullptr;
    QPushButton *m_addBtn = nullptr;
    QPushButton *m_editBtn = nullptr;
    QPushButton *m_removeBtn = nullptr;
    QLabel *m_statusLabel = nullptr;
    QLabel *m_familyHeadLabel = nullptr;
};

#endif // FAMILYVIEWERWIDGET_H
