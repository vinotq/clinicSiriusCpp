#ifndef PATIENTMANAGEMENTDIALOG_H
#define PATIENTMANAGEMENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "common/datamanager.h"
#include "patients/createpatientdialog.h"

class PatientManagementDialog : public QDialog {
    Q_OBJECT
public:
    explicit PatientManagementDialog(QWidget* parent = nullptr);

private slots:
    void onSearchTextChanged(const QString &text);
    void onCreatePatient();
    void onEditPatient();
    void onDeletePatient();
    void onAddToFamily();
    void refreshList(const QString &filter = QString());

private:
    DataManager m_dataManager;
    QLineEdit* m_searchEdit;
    QListWidget* m_list;
    QPushButton* m_createBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_addToFamilyBtn;
};

#endif // PATIENTMANAGEMENTDIALOG_H
