#ifndef PATIENTHISTORYWIDGET_H
#define PATIENTHISTORYWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include "../common/datamanager.h"

class PatientHistoryWidget : public QWidget {
    Q_OBJECT
public:
    explicit PatientHistoryWidget(QWidget *parent = nullptr);

private slots:
    void onSearchClicked();
    void onPatientSelected(QListWidgetItem *item);
    void onAppointmentSelected(QListWidgetItem *item);

private:
    DataManager m_dataManager;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QListWidget *m_patientsList;
    QListWidget *m_appointmentsList;
};

#endif // PATIENTHISTORYWIDGET_H
