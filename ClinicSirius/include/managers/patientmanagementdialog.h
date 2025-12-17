#ifndef PATIENTMANAGEMENTDIALOG_H
#define PATIENTMANAGEMENTDIALOG_H

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QCompleter>
#include "common/datamanager.h"
#include "patients/createpatientdialog.h"

class PatientManagementDialog : public QWidget {
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
    void buildUI();
    void showAddToFamilyDialog();

    DataManager m_dataManager;
    QLineEdit* m_searchEdit;
    QTableWidget* m_patientTable;
    QPushButton* m_createBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_addToFamilyBtn;
    QCompleter* m_patientNameCompleter;
};

#endif // PATIENTMANAGEMENTDIALOG_H
