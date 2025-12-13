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

// ========== SpecialtyCard ==========

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

    setStyleSheet(
        "SpecialtyCard {"
        "  border: 2px solid #e0e0e0;"
        "  border-radius: 8px;"
        "  background-color: white;"
        "}"
        "SpecialtyCard:hover {"
        "  border: 2px solid #4CAF50;"
        "  background-color: #f9fff9;"
        "}"
    );
    setMinimumSize(150, 120);
    setCursor(Qt::PointingHandCursor);
}

void SpecialtyCard::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    emit clicked();
}

// ========== DoctorCard ==========

DoctorCard::DoctorCard(int id, const QString& name, QWidget* parent)
    : QWidget(parent), m_id(id), m_name(name) {
    setupUI();
}

void DoctorCard::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* iconLabel = new QLabel("👨‍⚕️");
    QFont iconFont;
    iconFont.setPointSize(24);
    iconLabel->setFont(iconFont);
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

    setStyleSheet(
        "DoctorCard {"
        "  border: 2px solid #e0e0e0;"
        "  border-radius: 8px;"
        "  background-color: white;"
        "}"
        "DoctorCard:hover {"
        "  border: 2px solid #2196F3;"
        "  background-color: #f0f7ff;"
        "}"
    );
    setMinimumSize(140, 160);
    setCursor(Qt::PointingHandCursor);
}

void DoctorCard::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    emit clicked();
}

// ========== AppointmentBookingWidget ==========

AppointmentBookingWidget::AppointmentBookingWidget(QWidget* parent)
    : QWidget(parent), m_dataManager(QString()) {
    setupUI();
}

void AppointmentBookingWidget::setUser(const LoginUser &user) {
    m_currentUser = user;
}

void AppointmentBookingWidget::setInitialSelection(int doctorId, int scheduleId) {
    if (doctorId > 0) m_selectedDoctorId = doctorId;
    if (scheduleId > 0) {
        AppointmentSchedule sch = m_dataManager.getScheduleById(scheduleId);
        if (sch.id_ap_sch > 0) {
            m_selectedScheduleId = sch.id_ap_sch;
            m_selectedDateTime = sch.time_from;
            // Jump directly to patient selection for quick booking
            showPatientSelection();
            return;
        }
    }

    if (m_selectedDoctorId > 0) {
        showSlotSelection();
    }
}

AppointmentBookingWidget::~AppointmentBookingWidget() = default;

void AppointmentBookingWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 20, 30, 20);

    // Заголовок с кнопкой "Назад"
    QHBoxLayout* headerLayout = new QHBoxLayout();
    m_backButton = new QPushButton("← Назад");
    m_backButton->setMaximumWidth(100);
    m_backButton->setStyleSheet(
        "QPushButton {"
        "  border: 1px solid #ccc;"
        "  border-radius: 4px;"
        "  padding: 8px 12px;"
        "  background-color: white;"
        "}"
        "QPushButton:hover { background-color: #f5f5f5; }"
    );
    m_titleLabel = new QLabel("Запись к врачу");
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 18px;");
    headerLayout->addWidget(m_backButton);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();

    // Stacked Widget для показа разных этапов
    m_stackedWidget = new QStackedWidget();

    // Этап 1: Выбор специальности
    QWidget* specialtyPage = new QWidget();
    QVBoxLayout* specialtyLayout = new QVBoxLayout(specialtyPage);
    specialtyLayout->setContentsMargins(0, 0, 0, 0);
    specialtyLayout->addWidget(new QLabel("Выберите специальность:"));
    QGridLayout* specialtyGridLayout = new QGridLayout();
    specialtyGridLayout->setSpacing(15);

    QList<Specialization> specs = m_dataManager.getAllSpecializations();
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
    specialtyScroll->setStyleSheet("QScrollArea { border: none; }");
    m_stackedWidget->addWidget(specialtyScroll);

    // Этап 2: Выбор врача
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
    doctorScroll->setStyleSheet("QScrollArea { border: none; }");
    m_stackedWidget->addWidget(doctorScroll);

    // Этап 3: Выбор даты и времени
    QWidget* slotPage = new QWidget();
    QVBoxLayout* slotLayout = new QVBoxLayout(slotPage);
    slotLayout->setContentsMargins(0, 0, 0, 0);

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
    QPushButton* slotOkButton = new QPushButton("✓ Выбрать");
    slotOkButton->setObjectName("slotOkButton");
    QPushButton* slotCancelButton = new QPushButton("✗ Отмена");
    slotCancelButton->setObjectName("slotCancelButton");
    slotButtonLayout->addStretch();
    slotButtonLayout->addWidget(slotOkButton);
    slotButtonLayout->addWidget(slotCancelButton);
    slotLayout->addLayout(slotButtonLayout);

    m_stackedWidget->addWidget(slotPage);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_stackedWidget);

    // Подключение сигналов
    connect(m_backButton, &QPushButton::clicked, this, &AppointmentBookingWidget::onBackClicked);

    m_stackedWidget->setCurrentIndex(0);
}

void AppointmentBookingWidget::onSpecialtySelected(int specialtyId) {
    m_selectedSpecialtyId = specialtyId;
    m_titleLabel->setText("Выбор врача");

    // Очищаем и заполняем врачей
    auto doctorPage = m_stackedWidget->widget(1);
    auto doctorGridLayout = doctorPage->findChild<QGridLayout*>("doctorGridLayout");

    // Очищаем старые карточки
    QLayoutItem* item;
    while ((item = doctorGridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Добавляем новые карточки врачей
    QList<Doctor> allDoctors = m_dataManager.getAllDoctors();
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

    auto slotPage = m_stackedWidget->widget(2);
    auto calendar = slotPage->findChild<AppointmentCalendar*>("appointmentCalendar");
    auto slotsList = slotPage->findChild<QListWidget*>("appointmentSlots");
    auto slotOkButton = slotPage->findChild<QPushButton*>("slotOkButton");
    auto slotCancelButton = slotPage->findChild<QPushButton*>("slotCancelButton");

    if (!calendar || !slotsList || !slotOkButton || !slotCancelButton) {
        QMessageBox::critical(this, "Ошибка", "Не удалось инициализировать календарь");
        return;
    }

    // Загружаем доступные окна врача (только свободные и будущие)
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

    // Функция для загрузки слотов для выбранной даты
    auto loadSlots = [this, slotsList, availableSchedules](const QDate& date) {
        slotsList->clear();

        qDebug() << "Loading slots for date:" << date.toString("yyyy-MM-dd");
        
        // Сортируем расписание по времени
        QList<AppointmentSchedule> slotsForDate;
        for (const auto& schedule : availableSchedules) {
            if (schedule.time_from.date() == date) {
                slotsForDate.append(schedule);
            }
        }

        qDebug() << "Found" << slotsForDate.size() << "slots for this date";

        // Сортируем по времени
        std::sort(slotsForDate.begin(), slotsForDate.end(),
                  [](const AppointmentSchedule& a, const AppointmentSchedule& b) {
                      return a.time_from < b.time_from;
                  });

        for (const auto& schedule : slotsForDate) {
            QString timeStr = schedule.time_from.toString("HH:mm");
            auto item = new QListWidgetItem(timeStr);
            item->setData(Qt::UserRole, schedule.time_from.toString(Qt::ISODate));
            item->setData(Qt::UserRole + 1, schedule.id_ap_sch);  // Сохраняем id расписания
            item->setSizeHint(QSize(0, 35));
            slotsList->addItem(item);
        }
    };

    // Загружаем слоты для выбранной даты
    loadSlots(calendar->selectedDate());

    // Отключаем старые подключения
    disconnect(calendar, nullptr, nullptr, nullptr);
    disconnect(slotOkButton, nullptr, nullptr, nullptr);
    disconnect(slotCancelButton, nullptr, nullptr, nullptr);

    // Новые подключения
    connect(calendar, &AppointmentCalendar::selectionChanged, this, loadSlots);

    connect(slotOkButton, &QPushButton::clicked, this, [this, slotsList]() {
        auto item = slotsList->currentItem();
        if (!item) {
            QMessageBox::warning(this, "Ошибка", "Выберите время приема");
            return;
        }
        m_selectedDateTime = QDateTime::fromString(item->data(Qt::UserRole).toString(), Qt::ISODate);
        // Сохраняем id расписания, чтобы потом пометить слот как занятый
        m_selectedScheduleId = item->data(Qt::UserRole + 1).toInt();
        showPatientSelection();
    });

    connect(slotCancelButton, &QPushButton::clicked, this, &AppointmentBookingWidget::onBackClicked);

    m_stackedWidget->setCurrentIndex(2);
}

void AppointmentBookingWidget::showPatientSelection() {
    m_titleLabel->setText("Выбор пациента");
    QStringList patientNames;
    QList<Patient> patients;

    // If current user is a patient — allow selecting only from own family (and self), or create new
    if (m_currentUser.type == LoginUser::PATIENT) {
        QSet<int> addedIds;

        // include current user
        if (m_currentUser.id > 0) addedIds.insert(m_currentUser.id);

        // get family relations where current user is parent or child
        QList<PatientGroup> allMembers = m_dataManager.getPatientFamilyMembers(m_currentUser.id);
        QList<PatientGroup> parentGroups = m_dataManager.getPatientParents(m_currentUser.id);
        for (const PatientGroup &pg : parentGroups) allMembers.append(pg);

        // collect unique patient ids from family (parent->child or child->parent)
        for (const PatientGroup &pg : allMembers) {
            if (pg.id_parent == m_currentUser.id) {
                if (pg.id_child > 0) addedIds.insert(pg.id_child);
            } else if (pg.id_child == m_currentUser.id) {
                if (pg.id_parent > 0) addedIds.insert(pg.id_parent);
            }
        }

        // Convert to list and sort for deterministic order
        QList<int> idList = addedIds.values();
        std::sort(idList.begin(), idList.end());

        // build patients and names list
        for (int id : idList) {
            Patient p = m_dataManager.getPatientById(id);
            patients.append(p);
            QString name = p.fullName();
            if (name.trimmed().isEmpty()) name = QString("Пациент #%1").arg(id);
            patientNames.append(name);
        }

        // allow creating a new patient (will not implicitly join family)
        patientNames.append("+ Создать нового пациента");
    } else {
        // non-patient users can choose any patient
        patients = m_dataManager.getAllPatients();
        for (const auto& p : patients) {
            patientNames.append(p.fullName());
        }
        patientNames.append("+ Создать нового пациента");
    }

    bool ok;
    int index = -1;
    QString selected = QInputDialog::getItem(this, "Выбор пациента",
        "Выберите пациента:", patientNames, 0, false, &ok);

    if (!ok) {
        onBackClicked();
        return;
    }

    if (selected == "+ Создать нового пациента") {
        // Создание нового пациента
        bool nameOk, phoneOk;
        QString fname = QInputDialog::getText(this, "Новый пациент", "Имя:", QLineEdit::Normal, "", &nameOk);
        if (!nameOk) {
            onBackClicked();
            return;
        }

        QString lname = QInputDialog::getText(this, "Новый пациент", "Фамилия:", QLineEdit::Normal, "", &nameOk);
        if (!nameOk) {
            onBackClicked();
            return;
        }

        QString phone = QInputDialog::getText(this, "Новый пациент", "Телефон:", QLineEdit::Normal, "", &phoneOk);

        m_selectedPatient.id_patient = m_dataManager.getNextPatientId();
        m_selectedPatient.fname = fname;
        m_selectedPatient.lname = lname;
        m_selectedPatient.tname = "";
        m_selectedPatient.bdate = QDate::currentDate().toString("yyyy-MM-dd");
        m_selectedPatient.phone_number = phone;
        m_selectedPatient.email = "";
        m_selectedPatient.snils = "";
        m_selectedPatient.oms = "";
        m_selectedPatient.password = "";

        m_dataManager.addPatient(m_selectedPatient);
    } else {
        int idx = patientNames.indexOf(selected);
        if (idx >= 0 && idx < patients.size()) {
            m_selectedPatient = patients[idx];
        }
    }

    showConfirmation();
}

void AppointmentBookingWidget::showConfirmation() {
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
    Appointment appointment;
    appointment.id_ap = m_dataManager.getNextAppointmentId();
    appointment.id_patient = m_selectedPatient.id_patient;
    appointment.id_doctor = m_selectedDoctorId;
    appointment.date = m_selectedDateTime;
    appointment.id_ap_sch = m_selectedScheduleId; // привязать к слоту

    m_dataManager.addAppointment(appointment);

    // Пометить используемый слот как занятый
    if (m_selectedScheduleId > 0) {
        AppointmentSchedule sch = m_dataManager.getScheduleById(m_selectedScheduleId);
        if (sch.id_ap_sch > 0) {
            sch.status = "booked";
            m_dataManager.updateSchedule(sch);
        }
    }

    QMessageBox::information(this, "Успех",
        QString("Запись успешно создана!\n\nНомер приема: %1").arg(appointment.id_ap));

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
