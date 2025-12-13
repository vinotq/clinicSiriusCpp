#ifndef APPOINTMENTCALENDAR_H
#define APPOINTMENTCALENDAR_H

#include <QWidget>
#include <QDate>
#include <QSet>
#include <QGridLayout>

class AppointmentCalendar : public QWidget {
    Q_OBJECT

public:
    explicit AppointmentCalendar(QWidget* parent = nullptr);
    
    QDate selectedDate() const { return m_selectedDate; }
    void setSelectedDate(const QDate& date);
    void setAvailableDates(const QSet<QDate>& dates);
    void setMinimumDate(const QDate& date) { m_minDate = date; }

signals:
    void selectionChanged(const QDate& date);

private:
    void setupUI();
    void updateCalendar();
    void onDateClicked(const QDate& date);
    void onPrevMonth();
    void onNextMonth();
    
    QDate m_selectedDate;
    QDate m_displayedMonth;
    QDate m_minDate;
    QSet<QDate> m_availableDates;
    QGridLayout* m_gridLayout;
    
    static const int DAYS_IN_WEEK = 7;
};

#endif // APPOINTMENTCALENDAR_H
