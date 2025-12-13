#ifndef CREATEPATIENTDIALOG_H
#define CREATEPATIENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include "models.h"
#include "datamanager.h"

class CreatePatientDialog : public QDialog {
    Q_OBJECT
public:
    explicit CreatePatientDialog(QWidget *parent = nullptr, const Patient *existing = nullptr);
    Patient getCreatedPatient() const;

signals:
    void patientCreated(const Patient &patient);

private slots:
    void onCreatePatient();


private:
    void buildUI();

    bool editMode = false;
    int editingPatientId = 0;

    DataManager dataManager;
    Patient createdPatient;

    QLineEdit *firstNameEdit;
    QLineEdit *lastNameEdit;
    QLineEdit *middleNameEdit;
    QDateEdit *birthDateEdit;
    QLineEdit *phoneEdit;
    QLineEdit *emailEdit;
    QLineEdit *snilsEdit;
    QLineEdit *omsEdit;
    QLabel *statusLabel;
    QPushButton *createButton;
    QPushButton *cancelButton;
};

#endif // CREATEPATIENTDIALOG_H
