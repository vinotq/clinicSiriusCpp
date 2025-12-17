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
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    QLabel* iconLabel = new QLabel();
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(48, 48);
    QString iconPath = QString(":/images/icon-spec-%1.svg").arg((m_id % 10 == 0) ? 10 : (m_id % 10));
    QPixmap pm = QIcon(iconPath).pixmap(QSize(28, 28));
    if (!pm.isNull()) {
        iconLabel->setPixmap(pm);
    }
    layout->addWidget(iconLabel);

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
    layout->setContentsMargins(12, 12, 12, 12);

    QLabel* iconLabel = new QLabel();
    iconLabel->setProperty("class", "doctor-card-icon");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(52, 52);
    iconLabel->setPixmap(QIcon(":/images/icon-user.svg").pixmap(QSize(26, 26)));

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
    m_backButton = new QPushButton("Назад");
    m_backButton->setIcon(QIcon(":/images/icon-arrow-left.svg"));
    m_backButton->setIconSize(QSize(16, 16));
    m_backButton->setMaximumWidth(130);
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

    QHBoxLayout* specialtyButtonLayout = new QHBoxLayout();
    QPushButton* specialtyOkButton = new QPushButton("Далее");
    specialtyOkButton->setIcon(QIcon(":/images/icon-arrow-right.svg"));
    specialtyOkButton->setIconSize(QSize(16, 16));
    specialtyOkButton->setObjectName("specialtyOkButton");
    specialtyOkButton->setMinimumHeight(45);
    QPushButton* specialtyCancelButton = new QPushButton("Отмена");
    specialtyCancelButton->setIcon(QIcon(":/images/icon-close.svg"));
    specialtyCancelButton->setIconSize(QSize(16, 16));
    specialtyCancelButton->setObjectName("specialtyCancelButton");
    specialtyCancelButton->setMinimumHeight(45);
    specialtyButtonLayout->addStretch();
    specialtyButtonLayout->addWidget(specialtyOkButton);
    specialtyButtonLayout->addWidget(specialtyCancelButton);
    specialtyButtonLayout->setSpacing(10);
    specialtyLayout->addLayout(specialtyButtonLayout);

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
    doctorGridLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    doctorGridLayout->setColumnStretch(0, 1);
    doctorGridLayout->setColumnStretch(1, 1);
    doctorGridLayout->setColumnStretch(2, 1);
    doctorGridLayout->setObjectName("doctorGridLayout");
    doctorLayout->addLayout(doctorGridLayout);
    doctorLayout->addStretch();

    QHBoxLayout* doctorButtonLayout = new QHBoxLayout();
    QPushButton* doctorOkButton = new QPushButton("Далее");
    doctorOkButton->setIcon(QIcon(":/images/icon-arrow-right.svg"));
    doctorOkButton->setIconSize(QSize(16, 16));
    doctorOkButton->setObjectName("doctorOkButton");
    doctorOkButton->setMinimumHeight(45);
    QPushButton* doctorCancelButton = new QPushButton("Отмена");
    doctorCancelButton->setIcon(QIcon(":/images/icon-close.svg"));
    doctorCancelButton->setIconSize(QSize(16, 16));
    doctorCancelButton->setObjectName("doctorCancelButton");
    doctorCancelButton->setMinimumHeight(45);
    doctorButtonLayout->addStretch();
    doctorButtonLayout->addWidget(doctorOkButton);
    doctorButtonLayout->addWidget(doctorCancelButton);
    doctorButtonLayout->setSpacing(10);
    doctorLayout->addLayout(doctorButtonLayout);

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
    QPushButton* slotOkButton = new QPushButton("Выбрать");
    slotOkButton->setIcon(QIcon(":/images/icon-check.svg"));
    slotOkButton->setIconSize(QSize(16, 16));
    slotOkButton->setObjectName("slotOkButton");
    slotOkButton->setMinimumHeight(45);
    QPushButton* slotCancelButton = new QPushButton("Отмена");
    slotCancelButton->setIcon(QIcon(":/images/icon-close.svg"));
    slotCancelButton->setIconSize(QSize(16, 16));
    slotCancelButton->setObjectName("slotCancelButton");
    slotCancelButton->setMinimumHeight(45);
    slotButtonLayout->addStretch();
    slotButtonLayout->addWidget(slotOkButton);
    slotButtonLayout->addWidget(slotCancelButton);
    slotButtonLayout->setSpacing(10);
    slotLayout->addLayout(slotButtonLayout);

    m_stackedWidget->addWidget(slotPage);

    // Patient selection page (page 3)
    QWidget* patientPage = new QWidget();
    QVBoxLayout* patientLayout = new QVBoxLayout(patientPage);
    patientLayout->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* patientHeaderLayout = new QHBoxLayout();
    QLabel* patientLabel = new QLabel("Выберите пациента:");
    QPushButton* addPatientButton = new QPushButton("Создать пациента");
    addPatientButton->setIcon(QIcon(":/images/icon-add.svg"));
    addPatientButton->setIconSize(QSize(16,16));
    addPatientButton->setMinimumHeight(32);
    addPatientButton->setProperty("class", "hero-outline-btn");
    patientHeaderLayout->addWidget(patientLabel);
    patientHeaderLayout->addStretch();
    patientHeaderLayout->addWidget(addPatientButton);
    patientLayout->addLayout(patientHeaderLayout);
    QListWidget* patientsList = new QListWidget();
    patientsList->setObjectName("patientsList");
    patientLayout->addWidget(patientsList);

    QHBoxLayout* patientButtonLayout = new QHBoxLayout();
    QPushButton* patientOkButton = new QPushButton("Выбрать");
    patientOkButton->setIcon(QIcon(":/images/icon-check.svg"));
    patientOkButton->setIconSize(QSize(16, 16));
    patientOkButton->setObjectName("patientOkButton");
    patientOkButton->setMinimumHeight(45);
    QPushButton* patientCancelButton = new QPushButton("Отмена");
    patientCancelButton->setIcon(QIcon(":/images/icon-close.svg"));
    patientCancelButton->setIconSize(QSize(16, 16));
    patientCancelButton->setObjectName("patientCancelButton");
    patientCancelButton->setMinimumHeight(45);
    patientButtonLayout->addStretch();
    patientButtonLayout->addWidget(patientOkButton);
    patientButtonLayout->addWidget(patientCancelButton);
    patientButtonLayout->setSpacing(10);
    patientLayout->addLayout(patientButtonLayout);

    m_stackedWidget->addWidget(patientPage);

    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(m_stackedWidget);

    connect(m_backButton, &QPushButton::clicked, this, &AppointmentBookingWidget::onBackClicked);

    // Connect specialty page buttons
    QWidget* specPageWidget = m_stackedWidget->widget(0);
    QScrollArea* specPageScroll = qobject_cast<QScrollArea*>(specPageWidget);
    if (specPageScroll) {
        auto scrolledWidget = specPageScroll->widget();
        auto specialtyOkButton = scrolledWidget->findChild<QPushButton*>("specialtyOkButton");
        auto specialtyCancelButton = scrolledWidget->findChild<QPushButton*>("specialtyCancelButton");
        if (specialtyOkButton && specialtyCancelButton) {
            connect(specialtyOkButton, &QPushButton::clicked, this, [this]() {
                if (m_selectedSpecialtyId > 0) {
                    onSpecialtySelected(m_selectedSpecialtyId);
                }
            });
            connect(specialtyCancelButton, &QPushButton::clicked, this, [this]() {
                resetBooking();
                close();
            });
        }
    }

    // Connect doctor page buttons
    QWidget* docPageWidget = m_stackedWidget->widget(1);
    QScrollArea* docPageScroll = qobject_cast<QScrollArea*>(docPageWidget);
    if (docPageScroll) {
        auto scrolledWidget = docPageScroll->widget();
        auto doctorOkButton = scrolledWidget->findChild<QPushButton*>("doctorOkButton");
        auto doctorCancelButton = scrolledWidget->findChild<QPushButton*>("doctorCancelButton");
        if (doctorOkButton && doctorCancelButton) {
            connect(doctorOkButton, &QPushButton::clicked, this, [this]() {
                if (m_selectedDoctorId > 0) {
                    onDoctorSelected(m_selectedDoctorId);
                }
            });
            connect(doctorCancelButton, &QPushButton::clicked, this, &AppointmentBookingWidget::onBackClicked);
        }
    }

    // Connect patient page buttons
    QWidget* patientPageWidget = m_stackedWidget->widget(3);
    auto patientPgOkBtn = patientPageWidget->findChild<QPushButton*>("patientOkButton");
    auto patientPgCancelBtn = patientPageWidget->findChild<QPushButton*>("patientCancelButton");
    if (patientPgOkBtn && patientPgCancelBtn) {
        connect(patientPgOkBtn, &QPushButton::clicked, this, [this]() {
            auto patientPageWidget = m_stackedWidget->widget(3);
            auto patientsList = patientPageWidget->findChild<QListWidget*>("patientsList");
            auto item = patientsList->currentItem();
            if (!item) {
                QMessageBox::warning(this, "Ошибка", "Выберите пациента");
                return;
            }
            int patientId = item->data(Qt::UserRole).toInt();
            m_selectedPatient = m_dataManager.getPatientById(patientId);
            if (m_selectedPatient.id_patient > 0) {
                showConfirmation();
            }
        });
        connect(patientPgCancelBtn, &QPushButton::clicked, this, &AppointmentBookingWidget::onBackClicked);
    }

    // Создать пациента прямо из выбора
    connect(addPatientButton, &QPushButton::clicked, this, [this, patientsList]() {
        CreatePatientDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            Patient p = dlg.getCreatedPatient();
            if (p.id_patient > 0) {
                m_dataManager.addPatient(p);

                // Автодобавление в семью текущего пациента (если инициатор — пациент)
                if (m_currentUser.type == LoginUser::PATIENT && m_currentUser.id > 0) {
                    PatientGroup pg;
                    pg.id_patient_group = m_dataManager.getNextPatientGroupId();
                    pg.id_parent = m_currentUser.id;
                    pg.id_child = p.id_patient;
                    pg.family_head = m_currentUser.id;
                    m_dataManager.addFamilyMember(pg);
                }

                auto item = new QListWidgetItem(p.fullName());
                item->setData(Qt::UserRole, p.id_patient);
                patientsList->addItem(item);
                patientsList->setCurrentItem(item);
            }
        }
    });

    m_stackedWidget->setCurrentIndex(0);
}

void AppointmentBookingWidget::onSpecialtySelected(int specialtyId) {
    m_selectedSpecialtyId = specialtyId;
    m_titleLabel->setText("Выбор врача");
    m_progressLabel->setText("Шаг 2/5");

    auto doctorPage = m_stackedWidget->widget(1);
    auto doctorScroll = qobject_cast<QScrollArea*>(doctorPage);
    if (!doctorScroll) return;
    
    auto scrolledWidget = doctorScroll->widget();
    auto doctorGridLayout = scrolledWidget->findChild<QGridLayout*>("doctorGridLayout");
    if (!doctorGridLayout) return;

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

    if (availableSchedules.isEmpty()) {
        QMessageBox::information(this, "Нет свободных слотов",
                                 "У выбранного врача сейчас нет доступных приёмов. "
                                 "Попробуйте выбрать другого врача или вернитесь позже.");
        m_stackedWidget->setCurrentIndex(1);
        m_titleLabel->setText("Выбор врача");
        m_progressLabel->setText("Шаг 2/5");
        return;
    }

    for (const auto& schedule : availableSchedules) {
        datesWithSlots.insert(schedule.time_from.date());
        qDebug() << "  Adding date:" << schedule.time_from.date().toString("yyyy-MM-dd") << "Time:" << schedule.time_from.toString("HH:mm");
    }

    qDebug() << "Total dates with slots:" << datesWithSlots.size();
    
    calendar->reset();
    calendar->setMinimumDate(QDate::currentDate());
    calendar->setAvailableDates(datesWithSlots);

    // Select the first available date >= current date
    if (!datesWithSlots.isEmpty()) {
        QDate currentDate = QDate::currentDate();
        QDate firstValidDate;
        
        for (const auto& date : datesWithSlots) {
            if (date >= currentDate) {
                if (!firstValidDate.isValid() || date < firstValidDate) {
                    firstValidDate = date;
                }
            }
        }
        
        if (firstValidDate.isValid()) {
            calendar->setSelectedDate(firstValidDate);
        } else {
            calendar->setSelectedDate(QDate::currentDate());
        }
    } else {
        calendar->setSelectedDate(QDate::currentDate());
    }

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

    // Fill the patients list
    auto patientPageWidget = m_stackedWidget->widget(3);
    auto patientsList = patientPageWidget->findChild<QListWidget*>("patientsList");
    if (patientsList) {
        patientsList->clear();
        for (const auto& patient : availablePatients) {
            auto item = new QListWidgetItem(patient.fullName());
            item->setData(Qt::UserRole, patient.id_patient);
            patientsList->addItem(item);
        }
    }

    m_stackedWidget->setCurrentIndex(3);
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
            m_progressLabel->setText("Шаг 1/5");
        } else if (m_stackedWidget->currentIndex() == 1) {
            m_titleLabel->setText("Выбор врача");
            m_progressLabel->setText("Шаг 2/5");
        } else if (m_stackedWidget->currentIndex() == 2) {
            m_titleLabel->setText("Выбор даты и времени приема");
            m_progressLabel->setText("Шаг 3/5");
        }
    }
}

void AppointmentBookingWidget::resetBooking() {
    m_selectedSpecialtyId = -1;
    m_selectedDoctorId = -1;
    m_selectedScheduleId = -1;
    m_selectedDateTime = QDateTime();
    m_selectedPatient = Patient();
    m_isRescheduleMode = false;
    m_rescheduleAppointmentId = -1;
    m_oldScheduleId = -1;
    m_stackedWidget->setCurrentIndex(0);
    m_titleLabel->setText("Запись к врачу");
    m_progressLabel->setText("Шаг 1/5");
}
