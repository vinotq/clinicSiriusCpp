#ifndef PATIENTSELECTIONDIALOG_H
#define PATIENTSELECTIONDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "common/datamanager.h"
#include "common/models.h"

class PatientSelectionDialog : public QDialog {
    Q_OBJECT
public:
    explicit PatientSelectionDialog(QWidget *parent = nullptr, const QList<Patient> &availablePatients = QList<Patient>());
    Patient getSelectedPatient() const;

private slots:
    void onSearchTextChanged(const QString &text);
    void onCreateNew();
    void onListItemSelected(QListWidgetItem *item);

private:
    void buildUI();
    void updatePatientList(const QString &filter = QString());

    DataManager m_dataManager;
    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_patientList = nullptr;
    QPushButton *m_createBtn = nullptr;
    QPushButton *m_selfBtn = nullptr;
    QPushButton *m_selectBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;
    QLabel *m_statusLabel = nullptr;
    Patient m_selectedPatient;
    QList<Patient> m_allPatients;
};

#endif // PATIENTSELECTIONDIALOG_H
