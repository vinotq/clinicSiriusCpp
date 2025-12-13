#include "appointmentcalendar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QFont>
#include <QStyleOption>
#include <QPainter>

// Кнопка для дня
class DayButton : public QPushButton {
public:
    DayButton(const QDate& date, QWidget* parent = nullptr)
        : QPushButton(parent), m_date(date), m_isAvailable(false), m_isSelected(false) {
        setText(QString::number(date.day()));
        setMinimumSize(40, 40);
        setMaximumSize(50, 50);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::StrongFocus);
        updateStyle();
    }

    QDate date() const { return m_date; }
    
    void setAvailable(bool available) {
        m_isAvailable = available;
        updateStyle();
    }
    
    void setSelected(bool selected) {
        m_isSelected = selected;
        updateStyle();
    }

private:
    void updateStyle() {
        if (m_isSelected) {
            // Выбранный день - темный синий
            setEnabled(true);
            setStyleSheet(
                "QPushButton {"
                "  background-color: #2196F3;"
                "  color: white;"
                "  border: 2px solid #1976D2;"
                "  border-radius: 4px;"
                "  font-weight: bold;"
                "  padding: 5px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #1976D2;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #1565C0;"
                "}"
            );
        } else if (m_isAvailable) {
            // День с доступными слотами - светлый синий
            setEnabled(true);
            setStyleSheet(
                "QPushButton {"
                "  background-color: #E3F2FD;"
                "  color: #1565C0;"
                "  border: 2px solid #2196F3;"
                "  border-radius: 4px;"
                "  font-weight: bold;"
                "  padding: 5px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #BBDEFB;"
                "  border: 2px solid #1976D2;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #90CAF9;"
                "}"
            );
        } else {
            // День без доступных слотов - серый
            setEnabled(false);
            setStyleSheet(
                "QPushButton {"
                "  background-color: #F5F5F5;"
                "  color: #9E9E9E;"
                "  border: 1px solid #E0E0E0;"
                "  border-radius: 4px;"
                "  padding: 5px;"
                "}"
                "QPushButton:disabled {"
                "  background-color: #F5F5F5;"
                "  color: #BDBDBD;"
                "}"
            );
        }
    }

    QDate m_date;
    bool m_isAvailable;
    bool m_isSelected;
};

AppointmentCalendar::AppointmentCalendar(QWidget* parent)
    : QWidget(parent) {
    m_selectedDate = QDate::currentDate();
    m_displayedMonth = QDate::currentDate();
    m_minDate = QDate::currentDate();
    setupUI();
}

void AppointmentCalendar::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // Навигация по месяцам
    QHBoxLayout* navLayout = new QHBoxLayout();
    
    QPushButton* prevBtn = new QPushButton("←");
    prevBtn->setMaximumWidth(40);
    connect(prevBtn, &QPushButton::clicked, this, &AppointmentCalendar::onPrevMonth);
    
    QLabel* monthLabel = new QLabel();
    monthLabel->setAlignment(Qt::AlignCenter);
    monthLabel->setObjectName("monthLabel");
    QFont monthFont;
    monthFont.setPointSize(12);
    monthFont.setBold(true);
    monthLabel->setFont(monthFont);
    
    QPushButton* nextBtn = new QPushButton("→");
    nextBtn->setMaximumWidth(40);
    connect(nextBtn, &QPushButton::clicked, this, &AppointmentCalendar::onNextMonth);
    
    navLayout->addWidget(prevBtn);
    navLayout->addStretch();
    navLayout->addWidget(monthLabel);
    navLayout->addStretch();
    navLayout->addWidget(nextBtn);
    
    mainLayout->addLayout(navLayout);

    // Заголовки дней недели
    QStringList days = {"Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
    QHBoxLayout* daysHeaderLayout = new QHBoxLayout();
    daysHeaderLayout->setSpacing(5);
    
    for (const auto& day : days) {
        QLabel* dayLabel = new QLabel(day);
        dayLabel->setAlignment(Qt::AlignCenter);
        QFont dayFont;
        dayFont.setBold(true);
        dayLabel->setFont(dayFont);
        dayLabel->setMinimumHeight(30);
        daysHeaderLayout->addWidget(dayLabel);
    }
    
    mainLayout->addLayout(daysHeaderLayout);

    // Сетка для дней
    m_gridLayout = new QGridLayout();
    m_gridLayout->setSpacing(5);
    mainLayout->addLayout(m_gridLayout);

    mainLayout->addStretch();

    updateCalendar();
}

void AppointmentCalendar::updateCalendar() {
    // Очищаем старую сетку
    QLayoutItem* item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    auto monthLabel = findChild<QLabel*>("monthLabel");
    if (monthLabel) {
        QString monthName;
        switch (m_displayedMonth.month()) {
            case 1: monthName = "Январь"; break;
            case 2: monthName = "Февраль"; break;
            case 3: monthName = "Март"; break;
            case 4: monthName = "Апрель"; break;
            case 5: monthName = "Май"; break;
            case 6: monthName = "Июнь"; break;
            case 7: monthName = "Июль"; break;
            case 8: monthName = "Август"; break;
            case 9: monthName = "Сентябрь"; break;
            case 10: monthName = "Октябрь"; break;
            case 11: monthName = "Ноябрь"; break;
            case 12: monthName = "Декабрь"; break;
        }
        monthLabel->setText(QString("%1 %2").arg(monthName).arg(m_displayedMonth.year()));
    }

    // Первый день месяца
    QDate firstDay(m_displayedMonth.year(), m_displayedMonth.month(), 1);
    int startCol = (firstDay.dayOfWeek() == 7 ? 6 : firstDay.dayOfWeek() - 1);

    // Добавляем пустые ячейки в начало
    for (int i = 0; i < startCol; ++i) {
        QLabel* emptyLabel = new QLabel();
        emptyLabel->setMinimumSize(40, 40);
        m_gridLayout->addWidget(emptyLabel, 0, i);
    }

    // Добавляем дни месяца
    int row = 0;
    int col = startCol;
    QDate date = firstDay;
    
    while (date.month() == m_displayedMonth.month()) {
        DayButton* btn = new DayButton(date);
        bool isToday = (date == QDate::currentDate());
        bool availableVisual = (m_availableDates.contains(date) || isToday) && date >= m_minDate;
        btn->setAvailable(availableVisual);
        btn->setSelected(date == m_selectedDate);
        btn->setMinimumSize(40, 40);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFocusPolicy(Qt::StrongFocus);

        connect(btn, &QPushButton::clicked, this, [this, date]() {
            onDateClicked(date);
        });

        m_gridLayout->addWidget(btn, row, col);

        date = date.addDays(1);
        col++;
        if (col >= DAYS_IN_WEEK) {
            col = 0;
            row++;
        }
    }
}

void AppointmentCalendar::setSelectedDate(const QDate& date) {
    if (date.isValid() && date >= m_minDate) {
        m_selectedDate = date;
        if (date.year() != m_displayedMonth.year() || date.month() != m_displayedMonth.month()) {
            m_displayedMonth = QDate(date.year(), date.month(), 1);
        }
        updateCalendar();
        emit selectionChanged(date);
    }
}

void AppointmentCalendar::setAvailableDates(const QSet<QDate>& dates) {
    m_availableDates = dates;
    updateCalendar();
}

void AppointmentCalendar::onDateClicked(const QDate& date) {
    if (date >= m_minDate && m_availableDates.contains(date)) {
        setSelectedDate(date);
    }
}

void AppointmentCalendar::onPrevMonth() {
    m_displayedMonth = m_displayedMonth.addMonths(-1);
    if (m_displayedMonth < QDate(m_minDate.year(), m_minDate.month(), 1)) {
        m_displayedMonth = QDate(m_minDate.year(), m_minDate.month(), 1);
        return;
    }
    updateCalendar();
}

void AppointmentCalendar::onNextMonth() {
    m_displayedMonth = m_displayedMonth.addMonths(1);
    updateCalendar();
}
