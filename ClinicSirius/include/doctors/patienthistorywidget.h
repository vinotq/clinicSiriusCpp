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
    void onSearchTextChanged(const QString &text);
    void onPatientSelected(QListWidgetItem *item);
    void onAppointmentDoubleClicked(QListWidgetItem *item);

private:
    void populateCompleter();
    void openAppointmentDetails(int appointmentId);

    DataManager m_dataManager;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QListWidget *m_patientsList;
    QListWidget *m_appointmentsList;
    QCompleter *m_completer = nullptr;
};

#endif // PATIENTHISTORYWIDGET_H
