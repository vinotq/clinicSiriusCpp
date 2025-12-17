/* PatientAppointmentsViewer
 * Simple viewer for a patient's appointments with context menu actions
 */
#ifndef PATIENTAPPOINTMENTSVIEWER_H
#define PATIENTAPPOINTMENTSVIEWER_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include "common/datamanager.h"

class PatientAppointmentsViewer : public QWidget {
    Q_OBJECT
public:
    explicit PatientAppointmentsViewer(DataManager *dm, QWidget *parent = nullptr);
    void setCurrentPatient(int patientId);

private slots:
    void loadAppointmentsForPatient(int patientId);
    void onTableContextMenu(const QPoint &pos);
    void onAddAppointmentClicked(); // CHANGED: Slot for adding appointment

private:
    void buildUI();
    DataManager *m_dm;
    int m_currentPatientId = -1;
    QLineEdit *m_filterEdit = nullptr;
    QTableWidget *m_table = nullptr;
    QLabel *m_header = nullptr;
};

#endif // PATIENTAPPOINTMENTSVIEWER_H
