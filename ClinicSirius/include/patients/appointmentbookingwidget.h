#ifndef APPOINTMENTBOOKINGWIDGET_H
#define APPOINTMENTBOOKINGWIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QCalendarWidget>
#include <QListWidget>
#include "models.h"
#include "datamanager.h"

// Карточка для выбора специальности
class SpecialtyCard : public QWidget {
    Q_OBJECT
public:
    SpecialtyCard(int id, const QString& name, QWidget* parent = nullptr);
    int getId() const { return m_id; }

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void setupUI();
    int m_id;
    QString m_name;
};

// Карточка для выбора врача
class DoctorCard : public QWidget {
    Q_OBJECT
public:
    DoctorCard(int id, const QString& name, QWidget* parent = nullptr);
    int getId() const { return m_id; }

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void setupUI();
    int m_id;
    QString m_name;
};

class AppointmentBookingWidget : public QWidget {
    Q_OBJECT

public:
    explicit AppointmentBookingWidget(QWidget* parent = nullptr);
    ~AppointmentBookingWidget();
    void setUser(const LoginUser &user);
    // Preselect doctor and optionally a schedule id (then jump to patient selection)
    void setInitialSelection(int doctorId, int scheduleId = -1);
    // REQ-017: Set reschedule mode with appointment to reschedule
    void setRescheduleMode(int appointmentId, int doctorId);

private slots:
    void onSpecialtySelected(int specialtyId);
    void onDoctorSelected(int doctorId);
    void onBookingConfirmed();
    void onBackClicked();
    void resetBooking();

private:
    void setupUI();
    void showSpecialtySelection();
    void showDoctorSelection();
    void showSlotSelection();
    void showPatientSelection();
    void showConfirmation();

    DataManager m_dataManager;
    QStackedWidget* m_stackedWidget;
    QLabel* m_titleLabel;
    QLabel* m_progressLabel;
    QPushButton* m_backButton;

    // Booking state
    int m_selectedSpecialtyId = -1;
    int m_selectedDoctorId = -1;
    int m_selectedScheduleId = -1;
    QDateTime m_selectedDateTime;
    Patient m_selectedPatient;
    LoginUser m_currentUser;
    bool m_isFromManager = false;  // Track if booking is initiated from manager
    
    // REQ-017: Reschedule mode
    bool m_isRescheduleMode = false;
    int m_rescheduleAppointmentId = -1;
    int m_oldScheduleId = -1;
};

#endif // APPOINTMENTBOOKINGWIDGET_H
