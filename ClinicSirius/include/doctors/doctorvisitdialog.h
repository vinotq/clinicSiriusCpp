#ifndef DOCTORVISITDIALOG_H
#define DOCTORVISITDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include "models.h"
#include "datamanager.h"

class DoctorVisitDialog : public QDialog {
    Q_OBJECT
public:
    // Mode: 0 = visit from schedule, 1 = book new appointment (show patient selection)
    // scheduleId can be -1 when creating new appointment; doctorId is required
    explicit DoctorVisitDialog(int doctorId, int scheduleId = -1, int mode = 0, QWidget *parent = nullptr);
    void setCurrentPatient(int patientId);

signals:
    void appointmentSaved();
    void visitCompleted();

private slots:
    void onSaveAppointment();
    void onFinishVisit();
    void onSelectDoctorForBooking();
    void onCancelBooking();

private:
    void buildVisitUI();
    void buildBookingUI();
    void loadPatients();
    void loadDoctors();
    void loadSchedulesForSelectedDoctor();

    int doctorId;
    int scheduleId;
    int mode; // 0 = visit, 1 = booking
    int currentAppointmentId = -1;

    DataManager dataManager;

    // Visit mode widgets
    QComboBox *patientCombo = nullptr;
    QComboBox *diagnosisCombo = nullptr;
    QTextEdit *complaintsEdit = nullptr;
    QTextEdit *recommendationsEdit = nullptr;
    QTextEdit *notesEdit = nullptr;
    QPushButton *saveButton = nullptr;
    QPushButton *finishButton = nullptr;

    // Booking mode widgets
    QComboBox *patientComboBooking = nullptr;
    QComboBox *doctorComboBooking = nullptr;
    QComboBox *scheduleComboBooking = nullptr;
    QPushButton *bookButton = nullptr;
    QPushButton *cancelBookingButton = nullptr;
    QLabel *bookingStatusLabel = nullptr;

    QList<AppointmentSchedule> availableSchedules;
};

#endif // DOCTORVISITDIALOG_H
