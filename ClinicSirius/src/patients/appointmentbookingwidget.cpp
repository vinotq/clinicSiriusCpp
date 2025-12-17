#include "appointmentbookingwidget.h"
#include "appointmentcalendar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollArea>
#include <QGridLayout>
#include <QListWidget>
#include <QInputDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QSet>
#include <QLabel>
#include <QColor>
#include <QFont>
#include <QDate>
#include <QSize>
#include <QDebug>
#include <algorithm>
#include <QIcon>
#include <QPixmap>
#include "patients/createpatientdialog.h"
#include "patients/patientselectiondialog.h"

SpecialtyCard::SpecialtyCard(int id, const QString& name, QWidget* parent)
    : QWidget(parent), m_id(id), m_name(name) {
    setupUI();
}

void SpecialtyCard::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* titleLabel = new QLabel(m_name);
    QFont font;
    font.setPointSize(13);
    font.setBold(true);
    titleLabel->setFont(font);
    titleLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->addStretch();
    setProperty("class", "specialty-card");
    setMinimumSize(150, 120);
    setCursor(Qt::PointingHandCursor);
}

void SpecialtyCard::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    emit clicked();
}

DoctorCard::DoctorCard(int id, const QString& name, QWidget* parent)
    : QWidget(parent), m_id(id), m_name(name) {
    setupUI();
}

void DoctorCard::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* iconLabel = new QLabel("👩\u200D⚕️");
    iconLabel->setProperty("class", "doctor-card-icon");
    QFont ic; ic.setPointSize(20); iconLabel->setFont(ic);
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel* nameLabel = new QLabel(m_name);
    QFont nameFont;
    nameFont.setPointSize(11);
    nameFont.setBold(true);
    nameLabel->setFont(nameFont);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);

    layout->addWidget(iconLabel);
    layout->addWidget(nameLabel);
    layout->addStretch();
    setProperty("class", "doctor-card");
    setMinimumSize(140, 160);
    setCursor(Qt::PointingHandCursor);
}

void DoctorCard::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    emit clicked();
}

AppointmentBookingWidget::AppointmentBookingWidget(QWidget* parent)
    : QWidget(parent), m_dataManager(QString()) {
    setupUI();
}

void AppointmentBookingWidget::setUser(const LoginUser &user) {
    m_currentUser = user;
    m_isFromManager = (user.type == LoginUser::MANAGER);
}

void AppointmentBookingWidget::setInitialSelection(int doctorId, int scheduleId) {
    if (doctorId > 0) m_selectedDoctorId = doctorId;
    if (scheduleId > 0) {
        AppointmentSchedule sch = m_dataManager.getScheduleById(scheduleId);
        if (sch.id_ap_sch > 0) {
            m_selectedScheduleId = sch.id_ap_sch;
            m_selectedDateTime = sch.time_from;
            showPatientSelection();
            return;
        }
    }

    if (m_selectedDoctorId > 0) {
        showSlotSelection();
    }
}

void AppointmentBookingWidget::setRescheduleMode(int appointmentId, int doctorId) {
    m_isRescheduleMode = true;
    m_rescheduleAppointmentId = appointmentId;
    m_selectedDoctorId = doctorId;
    
    Appointment apt = m_dataManager.getAppointmentById(appointmentId);
    if (apt.id_ap > 0) {
        m_selectedPatient = m_dataManager.getPatientById(apt.id_patient);
        m_oldScheduleId = apt.id_ap_sch;
    }
    
    m_titleLabel->setText("Перенос приема");
    showSlotSelection();
}

AppointmentBookingWidget::~AppointmentBookingWidget() = default;

void AppointmentBookingWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 20, 30, 20);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    m_backButton = new QPushButton("⬅️ Назад");
    m_backButton->setMaximumWidth(100);
    m_backButton->setProperty("class", "back-button");
    m_titleLabel = new QLabel("Запись к врачу");
    m_titleLabel->setProperty("class", "page-title");
    m_titleLabel->setWordWrap(true);
    m_progressLabel = new QLabel("Шаг 1/5");
    m_progressLabel->setProperty("class", "progress-label");
    m_progressLabel->setAlignment(Qt::AlignRight);
    headerLayout->addWidget(m_backButton);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_progressLabel);

    m_stackedWidget = new QStackedWidget();

    QWidget* specialtyPage = new QWidget();
    QVBoxLayout* specialtyLayout = new QVBoxLayout(specialtyPage);
    specialtyLayout->setContentsMargins(0, 0, 0, 0);
    specialtyLayout->addWidget(new QLabel("Выберите специальность:"));
    QGridLayout* specialtyGridLayout = new QGridLayout();
    specialtyGridLayout->setSpacing(15);

    QList<Specialization> specs = m_dataManager.getAllSpecializations();
    std::sort(specs.begin(), specs.end(), [](const Specialization &a, const Specialization &b){ return a.name.toLower() < b.name.toLower(); });
    int row = 0, col = 0;
    for (const auto& spec : specs) {
        auto card = new SpecialtyCard(spec.id_spec, spec.name);
        specialtyGridLayout->addWidget(card, row, col);
        connect(card, &SpecialtyCard::clicked, this, [this, id = spec.id_spec]() {
            onSpecialtySelected(id);
        });
        col++;
        if (col >= 4) {
            col = 0;
            row++;
        }
    }
    specialtyLayout->addLayout(specialtyGridLayout);
    specialtyLayout->addStretch();

    QScrollArea* specialtyScroll = new QScrollArea();
    specialtyScroll->setWidget(specialtyPage);
    specialtyScroll->setWidgetResizable(true);
    specialtyScroll->setProperty("class", "no-border-scrollarea");
    m_stackedWidget->addWidget(specialtyScroll);

    QWidget* doctorPage = new QWidget();
    QVBoxLayout* doctorLayout = new QVBoxLayout(doctorPage);
    doctorLayout->setContentsMargins(0, 0, 0, 0);
    doctorLayout->addWidget(new QLabel("Выберите врача:"));
    QGridLayout* doctorGridLayout = new QGridLayout();
    doctorGridLayout->setSpacing(15);
    doctorGridLayout->setObjectName("doctorGridLayout");
    doctorLayout->addLayout(doctorGridLayout);
    doctorLayout->addStretch();

    QScrollArea* doctorScroll = new QScrollArea();
    doctorScroll->setWidget(doctorPage);
    doctorScroll->setWidgetResizable(true);
    doctorScroll->setProperty("class", "no-border-scrollarea");
    m_stackedWidget->addWidget(doctorScroll);

    QWidget* slotPage = new QWidget();
    QVBoxLayout* slotLayout = new QVBoxLayout(slotPage);
    slotLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* slotInfoLabel = new QLabel();
    slotInfoLabel->setText("Серые слоты недоступны: прошедшие, занятые или завершенные приемы");
    slotInfoLabel->setProperty("class", "slot-info-label");
    slotInfoLabel->setWordWrap(true);
    slotLayout->addWidget(slotInfoLabel);

    QHBoxLayout* dateTimeLayout = new QHBoxLayout();
    AppointmentCalendar* calendar = new AppointmentCalendar();
    calendar->setObjectName("appointmentCalendar");

    QListWidget* slotsList = new QListWidget();
    slotsList->setObjectName("appointmentSlots");
    slotsList->setMaximumWidth(180);
    slotsList->setMinimumWidth(150);

    dateTimeLayout->addWidget(calendar, 1);
    dateTimeLayout->addWidget(slotsList);

    slotLayout->addLayout(dateTimeLayout);

    QHBoxLayout* slotButtonLayout = new QHBoxLayout();
    QPushButton* slotOkButton = new QPushButton("✅ Выбрать");
    slotOkButton->setObjectName("slotOkButton");
    QPushButton* slotCancelButton = new QPushButton("❌ Отмена");
    slotCancelButton->setObjectName("slotCancelButton");
    slotButtonLayout->addStretch();
    slotButtonLayout->addWidget(slotOkButton);
    slotButtonLayout->addWidget(slotCancelButton);
    slotLayout->addLayout(slotButtonLayout);

    m_stackedWidget->addWidget(slotPage);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_stackedWidget);

    connect(m_backButton, &QPushButton::clicked, this, &AppointmentBookingWidget::onBackClicked);

    m_stackedWidget->setCurrentIndex(0);
}

void AppointmentBookingWidget::onSpecialtySelected(int specialtyId) {
    m_selectedSpecialtyId = specialtyId;
    m_titleLabel->setText("Выбор врача");
    m_progressLabel->setText("Шаг 2/5");

    auto doctorPage = m_stackedWidget->widget(1);
    auto doctorGridLayout = doctorPage->findChild<QGridLayout*>("doctorGridLayout");

    QLayoutItem* item;
    while ((item = doctorGridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    QList<Doctor> allDoctors = m_dataManager.getAllDoctors();
    std::sort(allDoctors.begin(), allDoctors.end(), [](const Doctor &a, const Doctor &b){ return a.fullName().toLower() < b.fullName().toLower(); });
    int row = 0, col = 0;
    for (const auto& doctor : allDoctors) {
        if (doctor.id_spec == specialtyId) {
            QString displayName = QString("%1 %2").arg(doctor.lname, doctor.fname);
            auto card = new DoctorCard(doctor.id_doctor, displayName);
            doctorGridLayout->addWidget(card, row, col);
            connect(card, &DoctorCard::clicked, this, [this, id = doctor.id_doctor]() {
                onDoctorSelected(id);
            });
            col++;
            if (col >= 3) {
                col = 0;
                row++;
            }
        }
    }

    m_stackedWidget->setCurrentIndex(1);
}

void AppointmentBookingWidget::onDoctorSelected(int doctorId) {
    m_selectedDoctorId = doctorId;
    showSlotSelection();
}

void AppointmentBookingWidget::showSlotSelection() {
    m_titleLabel->setText("Выбор даты и времени приема");
    m_progressLabel->setText("Шаг 3/5");

    auto slotPage = m_stackedWidget->widget(2);
    auto calendar = slotPage->findChild<AppointmentCalendar*>("appointmentCalendar");
    auto slotsList = slotPage->findChild<QListWidget*>("appointmentSlots");
    auto slotOkButton = slotPage->findChild<QPushButton*>("slotOkButton");
    auto slotCancelButton = slotPage->findChild<QPushButton*>("slotCancelButton");

    if (!calendar || !slotsList || !slotOkButton || !slotCancelButton) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать календарь");
        return;
    }

    QList<AppointmentSchedule> availableSchedules = m_dataManager.getAvailableSchedules(m_selectedDoctorId);
    qDebug() << "showSlotSelection: Doctor" << m_selectedDoctorId << "has" << availableSchedules.size() << "available slots";
    
    QSet<QDate> datesWithSlots;

    for (const auto& schedule : availableSchedules) {
        datesWithSlots.insert(schedule.time_from.date());
        qDebug() << "  Adding date:" << schedule.time_from.date().toString("yyyy-MM-dd") << "Time:" << schedule.time_from.toString("HH:mm");
    }

    qDebug() << "Total dates with slots:" << datesWithSlots.size();
    
    calendar->setMinimumDate(QDate::currentDate());
    calendar->setAvailableDates(datesWithSlots);

    auto loadSlots = [this, slotsList, availableSchedules](const QDate& date) {
        slotsList->clear();

        qDebug() << "Loading slots for date:" << date.toString("yyyy-MM-dd");
        
        QList<AppointmentSchedule> slotsForDate;
        for (const auto& schedule : availableSchedules) {
            if (schedule.time_from.date() == date) {
                slotsForDate.append(schedule);
            }
        }

        qDebug() << "Found" << slotsForDate.size() << "slots for this date";

        std::sort(slotsForDate.begin(), slotsForDate.end(),
                  [](const AppointmentSchedule& a, const AppointmentSchedule& b) {
                      return a.time_from < b.time_from;
                  });

        QDateTime now = QDateTime::currentDateTime();

        for (const auto& schedule : slotsForDate) {
            QString timeStr = schedule.time_from.toString("HH:mm");
            auto item = new QListWidgetItem(timeStr);
            item->setData(Qt::UserRole, schedule.time_from.toString(Qt::ISODate));
            item->setData(Qt::UserRole + 1, schedule.id_ap_sch);
            item->setSizeHint(QSize(0, 35));

            bool isValid = true;
            QString invalidReason;

            if (schedule.time_from < now) {
                isValid = false;
                invalidReason = QString("%1 — прошедший прием").arg(timeStr);
            } else {
                QString status = schedule.status.trimmed().toLower();
                if (status == "booked" || status == "busy") {
                    isValid = false;
                    invalidReason = QString("%1 — занято").arg(timeStr);
                } else if (status == "done") {
                    isValid = false;
                    invalidReason = QString("%1 — завершено").arg(timeStr);
                }
            }

            if (isValid) {
                item->setText(timeStr);
            } else {
                item->setText(invalidReason);
                item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
                item->setForeground(QColor("#999999"));
            }

            slotsList->addItem(item);
        }
    };

    loadSlots(calendar->selectedDate());

    disconnect(calendar, nullptr, nullptr, nullptr);
    disconnect(slotOkButton, nullptr, nullptr, nullptr);
    disconnect(slotCancelButton, nullptr, nullptr, nullptr);

    connect(calendar, &AppointmentCalendar::selectionChanged, this, loadSlots);

    connect(slotOkButton, &QPushButton::clicked, this, [this, slotsList]() {
        auto item = slotsList->currentItem();
        if (!item) {
            QMessageBox::warning(this, "Ошибка", "Выберите время приема");
            return;
        }

        if (!(item->flags() & Qt::ItemIsSelectable)) {
            QMessageBox::warning(this, "Ошибка", 
                "Невозможно выбрать этот слот. " + item->text());
            return;
        }

        m_selectedDateTime = QDateTime::fromString(item->data(Qt::UserRole).toString(), Qt::ISODate);
        m_selectedScheduleId = item->data(Qt::UserRole + 1).toInt();
        
        AppointmentSchedule sch = m_dataManager.getScheduleById(m_selectedScheduleId);
        if (sch.id_ap_sch <= 0) {
            QMessageBox::warning(this, "Ошибка", "Слот не найден в системе");
            return;
        }
        
        QDateTime now = QDateTime::currentDateTime();
        if (sch.time_from < now) {
            QMessageBox::warning(this, "Ошибка", 
                QString("Невозможно записаться на прошедший прием (%1)")
                    .arg(sch.time_from.toString("dd.MM.yyyy HH:mm")));
            return;
        }
        
        QString status = sch.status.trimmed().toLower();
        if (status == "booked" || status == "busy") {
            QMessageBox::warning(this, "Ошибка", 
                "Выбранный слот уже занят другим пациентом");
            return;
        }
        
        if (status == "done") {
            QMessageBox::warning(this, "Ошибка", 
                "Невозможно записаться на завершенный прием");
            return;
        }

        showPatientSelection();
    });

    connect(slotCancelButton, &QPushButton::clicked, this, &AppointmentBookingWidget::onBackClicked);

    m_stackedWidget->setCurrentIndex(2);
}

void AppointmentBookingWidget::showPatientSelection() {
    if (m_isRescheduleMode) {
        showConfirmation();
        return;
    }
    
    m_titleLabel->setText("Выбор пациента");
    m_progressLabel->setText("Шаг 4/5");
    QList<Patient> availablePatients;

    if (m_currentUser.type == LoginUser::PATIENT) {
        QSet<int> addedIds;

        if (m_currentUser.id > 0) addedIds.insert(m_currentUser.id);

        QList<PatientGroup> allMembers = m_dataManager.getPatientFamilyMembers(m_currentUser.id);
        QList<PatientGroup> parentGroups = m_dataManager.getPatientParents(m_currentUser.id);
        for (const PatientGroup &pg : parentGroups) allMembers.append(pg);

        for (const PatientGroup &pg : allMembers) {
            if (pg.id_parent == m_currentUser.id) {
                if (pg.id_child > 0) addedIds.insert(pg.id_child);
            } else if (pg.id_child == m_currentUser.id) {
                if (pg.id_parent > 0) addedIds.insert(pg.id_parent);
            }
        }

        for (int id : addedIds.values()) {
            Patient p = m_dataManager.getPatientById(id);
            availablePatients.append(p);
        }
        std::sort(availablePatients.begin(), availablePatients.end(), [](const Patient &a, const Patient &b){ return a.fullName().toLower() < b.fullName().toLower(); });
    } else {
        availablePatients = m_dataManager.getAllPatients();
        std::sort(availablePatients.begin(), availablePatients.end(), [](const Patient &a, const Patient &b){ return a.fullName().toLower() < b.fullName().toLower(); });
    }

    PatientSelectionDialog dlg(this, availablePatients);
    if (dlg.exec() == QDialog::Accepted) {
        m_selectedPatient = dlg.getSelectedPatient();
        if (m_selectedPatient.id_patient > 0) {
            showConfirmation();
        } else {
            onBackClicked();
        }
    } else {
        if (m_isFromManager) {
            close();
        } else {
            onBackClicked();
        }
    }
}

void AppointmentBookingWidget::showConfirmation() {
    m_titleLabel->setText("Подтверждение записи");
    m_progressLabel->setText("Шаг 5/5");
    
    Doctor doctor = m_dataManager.getDoctorById(m_selectedDoctorId);
    Specialization spec = m_dataManager.getSpecializationById(doctor.id_spec);

    QString confirmText = QString(
        "Подтверждение записи:\n\n"
        "Специальность: %1\n"
        "Врач: %2 %3\n"
        "Дата и время: %4\n"
        "Пациент: %5\n\n"
        "Подтвердить запись?"
    ).arg(spec.name, doctor.fname, doctor.lname,
          m_selectedDateTime.toString("dd.MM.yyyy HH:mm"),
          m_selectedPatient.fullName());

    if (QMessageBox::question(this, "Подтверждение", confirmText) == QMessageBox::Yes) {
        onBookingConfirmed();
    } else {
        onBackClicked();
    }
}

void AppointmentBookingWidget::onBookingConfirmed() {
    if (m_isRescheduleMode) {
        Appointment appointment = m_dataManager.getAppointmentById(m_rescheduleAppointmentId);
        if (appointment.id_ap <= 0) {
            QMessageBox::warning(this, "Ошибка", "Запись не найдена");
            return;
        }
        
        if (m_oldScheduleId > 0) {
            AppointmentSchedule oldSch = m_dataManager.getScheduleById(m_oldScheduleId);
            if (oldSch.id_ap_sch > 0) {
                oldSch.status = "free";
                m_dataManager.updateSchedule(oldSch);
            }
        }
        
        appointment.date = m_selectedDateTime;
        appointment.id_ap_sch = m_selectedScheduleId;
        m_dataManager.updateAppointment(appointment);
        
        if (m_selectedScheduleId > 0) {
            AppointmentSchedule newSch = m_dataManager.getScheduleById(m_selectedScheduleId);
            if (newSch.id_ap_sch > 0) {
                newSch.status = "booked";
                m_dataManager.updateSchedule(newSch);
            }
        }
        
        QMessageBox::information(this, "Успех", "Запись перенесена на новое время");
    } else {
        Appointment appointment;
        appointment.id_ap = m_dataManager.getNextAppointmentId();
        appointment.id_patient = m_selectedPatient.id_patient;
        appointment.id_doctor = m_selectedDoctorId;
        appointment.date = m_selectedDateTime;
        appointment.id_ap_sch = m_selectedScheduleId;

        m_dataManager.addAppointment(appointment);

        if (m_selectedScheduleId > 0) {
            AppointmentSchedule sch = m_dataManager.getScheduleById(m_selectedScheduleId);
            if (sch.id_ap_sch > 0) {
                sch.status = "booked";
                m_dataManager.updateSchedule(sch);
            }
        }

        QMessageBox::information(this, "Успех",
            QString("Запись успешно создана!\n\nНомер приема: %1").arg(appointment.id_ap));
    }

    resetBooking();
}

void AppointmentBookingWidget::onBackClicked() {
    if (m_stackedWidget->currentIndex() > 0) {
        m_stackedWidget->setCurrentIndex(m_stackedWidget->currentIndex() - 1);
        if (m_stackedWidget->currentIndex() == 0) {
            m_titleLabel->setText("Запись к врачу");
        } else if (m_stackedWidget->currentIndex() == 1) {
            m_titleLabel->setText("Выбор врача");
        }
    }
}

void AppointmentBookingWidget::resetBooking() {
    m_selectedSpecialtyId = -1;
    m_selectedDoctorId = -1;
    m_selectedScheduleId = -1;
    m_selectedDateTime = QDateTime();
    m_stackedWidget->setCurrentIndex(0);
    m_titleLabel->setText("Запись к врачу");
}
