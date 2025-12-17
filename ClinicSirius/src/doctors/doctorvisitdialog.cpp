#include "doctorvisitdialog.h"
#include <QFormLayout>
#include <QDateTime>
#include <QMessageBox>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCompleter>
#include <QStringListModel>
#include <QIcon>

DoctorVisitDialog::DoctorVisitDialog(int doctorId_, int scheduleId_, int mode_, QWidget *parent)
    : QDialog(parent), doctorId(doctorId_), scheduleId(scheduleId_), mode(mode_),
      dataManager(QCoreApplication::applicationDirPath() + "/../data") {
    setWindowTitle("Окно приема");
    setMinimumWidth(500);
    
    if (mode == 0) {
        buildVisitUI();
        loadPatients();
    } else {
        buildBookingUI();
        loadPatients();  // Загрузить пациентов для режима записи (с поиском)
    }
}

void DoctorVisitDialog::buildVisitUI() {
    // Диалог для проведения приема (режим = 0)
    QVBoxLayout *main = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    QLabel *titleLabel = new QLabel("Прием пациента");
    titleLabel->setProperty("class", "dialog-title");
    main->addWidget(titleLabel);

    patientCombo = new QComboBox();
    diagnosisCombo = new QComboBox();
    complaintsEdit = new QTextEdit();
    recommendationsEdit = new QTextEdit();

    // Patient selector (non-editable here; booking uses searchable control)
    patientCombo->setEditable(false);

    form->addRow("Пациент:", patientCombo);
    form->addRow("Диагноз:", diagnosisCombo);
    form->addRow("Жалобы:", complaintsEdit);
    form->addRow("Рекомендации:", recommendationsEdit);

    // Populate diagnoses
    diagnosisCombo->clear();
    QList<Diagnosis> diags = dataManager.getAllDiagnoses();
    for (const Diagnosis &d : diags) {
        diagnosisCombo->addItem(d.name, d.id_diagnosis);
    }

    // Если открыли с конкретным расписанием (занятый слот), найти и выбрать пациента
    if (scheduleId >= 0) {
        QList<Appointment> appts = dataManager.getAppointmentsByDoctor(doctorId);
        for (const Appointment &a : appts) {
            if (a.id_ap_sch == scheduleId) {
                patientCombo->setEnabled(false);  // Пациент уже выбран, не менять
                currentAppointmentId = a.id_ap;
                break;
            }
        }
    }

    saveButton = new QPushButton("Записать на прием");
    saveButton->setIcon(QIcon(":/images/icon-calendar.svg"));
    saveButton->setIconSize(QSize(16,16));
    finishButton = new QPushButton("Завершить прием");
    finishButton->setIcon(QIcon(":/images/icon-check.svg"));
    finishButton->setIconSize(QSize(16,16));
    repeatBookButton = new QPushButton("Записать повторно");
    repeatBookButton->setIcon(QIcon(":/images/icon-refresh.svg"));
    repeatBookButton->setIconSize(QSize(16,16));

    QHBoxLayout *actions = new QHBoxLayout();
    // Убрали прямую запись; оставили только повторную (менеджерский сценарий)
    actions->addWidget(repeatBookButton);
    actions->addStretch();
    actions->addWidget(finishButton);

    main->addLayout(form);
    main->addLayout(actions);

    connect(finishButton, &QPushButton::clicked, this, &DoctorVisitDialog::onFinishVisit);
    connect(repeatBookButton, &QPushButton::clicked, this, &DoctorVisitDialog::onRepeatBooking);
}

void DoctorVisitDialog::buildBookingUI() {
    // Диалог для записи пациента на прием (режим = 1)
    QVBoxLayout *main = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    QLabel *titleLabel = new QLabel("Запись пациента на прием");
    titleLabel->setProperty("class", "dialog-title");
    main->addWidget(titleLabel);

    patientComboBooking = new QComboBox();
    // Make booking patient selector editable with completer for search
    patientComboBooking->setEditable(true);
    patientComboBooking->setInsertPolicy(QComboBox::NoInsert);
    doctorComboBooking = new QComboBox();
    scheduleComboBooking = new QComboBox();
    bookingStatusLabel = new QLabel("");

    form->addRow("Пациент:", patientComboBooking);
    form->addRow("Врач:", doctorComboBooking);
    form->addRow("Время приема:", scheduleComboBooking);

    loadDoctors();
    // populate patients for booking (searchable)
    QList<Patient> pats = dataManager.getAllPatients();
    QStringList names;
    for (const Patient &p : pats) {
        patientComboBooking->addItem(p.fullName(), p.id_patient);
        names << p.fullName();
    }
    QCompleter *completer = new QCompleter(names, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    patientComboBooking->setCompleter(completer);

    // Если scheduleId указан, загрузить расписания и выбрать нужный слот
    if (scheduleId >= 0) {
        doctorComboBooking->setEnabled(false);
        scheduleComboBooking->setEnabled(false);
        
        int doctorIdx = doctorComboBooking->findData(doctorId);
        if (doctorIdx >= 0) {
            doctorComboBooking->setCurrentIndex(doctorIdx);
        }
        
        loadSchedulesForSelectedDoctor();
        
        int scheduleIdx = scheduleComboBooking->findData(scheduleId);
        if (scheduleIdx >= 0) {
            scheduleComboBooking->setCurrentIndex(scheduleIdx);
        }
    } else {
        connect(doctorComboBooking, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &DoctorVisitDialog::loadSchedulesForSelectedDoctor);
    }
    
    bookButton = new QPushButton("Записать");
    bookButton->setIcon(QIcon(":/images/icon-check.svg"));
    bookButton->setIconSize(QSize(16,16));
    cancelBookingButton = new QPushButton("Отменить запись");
    cancelBookingButton->setIcon(QIcon(":/images/icon-close.svg"));
    cancelBookingButton->setIconSize(QSize(16,16));

    QHBoxLayout *actions = new QHBoxLayout();
    actions->addWidget(bookButton);
    actions->addStretch();
    actions->addWidget(cancelBookingButton);
    main->addLayout(form);
    main->addWidget(bookingStatusLabel);
    main->addLayout(actions);

    connect(bookButton, &QPushButton::clicked, this, &DoctorVisitDialog::onSaveAppointment);
    connect(cancelBookingButton, &QPushButton::clicked, this, &DoctorVisitDialog::onCancelBooking);
}

void DoctorVisitDialog::loadPatients() {
    if (mode == 0) {
        // Режим приема - заполняем patientCombo
        if (patientCombo) {
            patientCombo->clear();
            QList<Patient> pats = dataManager.getAllPatients();
            for (const Patient &p : pats) {
                patientCombo->addItem(p.fullName(), p.id_patient);
            }

            // Если пришли из расписания с уже занятого слота — выбрать пациента автоматически
            if (scheduleId >= 0) {
                QList<Appointment> appts = dataManager.getAppointmentsByDoctor(doctorId);
                for (const Appointment &a : appts) {
                    if (a.id_ap_sch == scheduleId) {
                        int idx = patientCombo->findData(a.id_patient);
                        if (idx >= 0) {
                            patientCombo->setCurrentIndex(idx);
                            patientCombo->setEnabled(false); // пациент фиксирован для этого слота
                        }
                        currentAppointmentId = a.id_ap;
                        break;
                    }
                }
            } else {
                patientCombo->setCurrentIndex(-1); // не выбирать по умолчанию
            }
        }
    } else {
        // Режим записи - заполняем patientComboBooking
        if (patientComboBooking) {
            // already populated in buildBookingUI; update entries in case of changes
            patientComboBooking->clear();
            QList<Patient> pats = dataManager.getAllPatients();
            QStringList names;
            for (const Patient &p : pats) {
                patientComboBooking->addItem(p.fullName(), p.id_patient);
                names << p.fullName();
            }
            if (patientComboBooking->completer()) {
                QCompleter *c = patientComboBooking->completer();
                c->setModel(new QStringListModel(names, c));
            } else {
                QCompleter *completer = new QCompleter(names, this);
                completer->setCaseSensitivity(Qt::CaseInsensitive);
                patientComboBooking->setCompleter(completer);
            }
            patientComboBooking->setCurrentIndex(-1); // не выбирать по умолчанию
        }
    }
}

void DoctorVisitDialog::loadDoctors() {
    doctorComboBooking->clear();
    QList<Doctor> docs = dataManager.getAllDoctors();
    for (const Doctor &d : docs) {
        doctorComboBooking->addItem(d.fullName(), d.id_doctor);
    }
    // По умолчанию выбрать текущего врача
    int idx = doctorComboBooking->findData(doctorId);
    if (idx >= 0) doctorComboBooking->setCurrentIndex(idx);
}

void DoctorVisitDialog::loadSchedulesForSelectedDoctor() {
    if (!scheduleComboBooking || !bookingStatusLabel) {
        return;
    }
    
    scheduleComboBooking->clear();
    int selectedDoctorId = doctorComboBooking->currentData().toInt();
    availableSchedules = dataManager.getAvailableSchedules(selectedDoctorId);
    
    for (const AppointmentSchedule &sch : availableSchedules) {
        QString displayText = QString("%1 %2 - %3")
            .arg(sch.time_from.date().toString("dd.MM.yyyy"),
                 sch.time_from.time().toString("HH:mm"),
                 sch.time_to.time().toString("HH:mm"));
        scheduleComboBooking->addItem(displayText, sch.id_ap_sch);
    }
    
    if (availableSchedules.isEmpty()) {
        bookingStatusLabel->setText("Нет свободных окон у выбранного врача");
    } else {
        bookingStatusLabel->setText("");
    }
}


void DoctorVisitDialog::setCurrentPatient(int patientId) {
    if (mode == 0 && patientCombo) {
        int idx = patientCombo->findData(patientId);
        if (idx >= 0) patientCombo->setCurrentIndex(idx);
    }
}

void DoctorVisitDialog::setBookingPatient(int patientId, bool lock) {
    if (mode == 1 && patientComboBooking) {
        int idx = patientComboBooking->findData(patientId);
        if (idx >= 0) {
            patientComboBooking->setCurrentIndex(idx);
            if (lock) patientComboBooking->setEnabled(false);
        }
    }
}

void DoctorVisitDialog::onSaveAppointment() {
    int patientId = -1;
    int doctorIdForBooking = doctorId;
    int scheduleIdUsed = -1;
    QDateTime appointmentTime;
    
    if (mode == 0) {
        // Режим приема
        if (!patientCombo) {
            QMessageBox::warning(this, "Ошибка", "Виджет пациента не инициализирован");
            return;
        }
        
        patientId = patientCombo->currentData().toInt();
        if (patientId <= 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите пациента");
            return;
        }

        if (scheduleId >= 0) {
            AppointmentSchedule sch = dataManager.getScheduleById(scheduleId);
            if (!sch.time_from.isValid()) {
                QMessageBox::warning(this, "Ошибка", "Не удалось загрузить расписание");
                return;
            }
            appointmentTime = sch.time_from;
            scheduleIdUsed = scheduleId;
        } else {
            QList<AppointmentSchedule> avail = dataManager.getAvailableSchedules(doctorId);
            if (avail.isEmpty()) {
                QMessageBox::information(this, "Нет окон", "Нет свободных окон для записи");
                return;
            }
            appointmentTime = avail.first().time_from;
            scheduleIdUsed = avail.first().id_ap_sch;
        }
    } else {
        // Режим записи
        if (!patientComboBooking || !doctorComboBooking || !scheduleComboBooking) {
            QMessageBox::warning(this, "Ошибка", "Виджеты не инициализированы");
            return;
        }
        
        patientId = patientComboBooking->currentData().toInt();
        // If user typed a name (completer) but didn't select, try to find matching name with exact match first, then partial
        if (patientId <= 0) {
            QString txt = patientComboBooking->currentText().trimmed().toLower();
            if (!txt.isEmpty()) {
                int bestMatchIdx = -1;
                // First pass: exact case-insensitive match
                for (int i = 0; i < patientComboBooking->count(); ++i) {
                    QString itext = patientComboBooking->itemText(i).toLower();
                    if (itext == txt) {
                        patientId = patientComboBooking->itemData(i).toInt();
                        break;
                    } else if (itext.contains(txt) && bestMatchIdx == -1) {
                        bestMatchIdx = i; // Store first partial match
                    }
                }
                // If no exact match found, use partial match
                if (patientId <= 0 && bestMatchIdx != -1) {
                    patientId = patientComboBooking->itemData(bestMatchIdx).toInt();
                    patientComboBooking->setCurrentIndex(bestMatchIdx); // Auto-select
                }
            }
        }
        doctorIdForBooking = doctorComboBooking->currentData().toInt();
        
        if (patientId <= 0) {
            QMessageBox::warning(this, "Ошибка", "Пациент не найден. Выберите из списка или уточните фамилию.");
            return;
        }
        
        if (doctorIdForBooking <= 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите врача");
            return;
        }
        
        int scheduleIdBooking = scheduleComboBooking->currentData().toInt();
        if (scheduleIdBooking <= 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите время приема");
            return;
        }
        
        AppointmentSchedule sch = dataManager.getScheduleById(scheduleIdBooking);
        if (!sch.time_from.isValid()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить расписание");
            return;
        }
        appointmentTime = sch.time_from;
        scheduleIdUsed = scheduleIdBooking;
    }

    if (!appointmentTime.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Неверная дата и время приема");
        return;
    }

    // Проверки слота и существующей записи
    int existingApptId = -1;
    Appointment existingAppt;
    if (scheduleIdUsed > 0) {
        AppointmentSchedule sch = dataManager.getScheduleById(scheduleIdUsed);
        if (!sch.time_from.isValid()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить слот расписания.");
            return;
        }
        if (sch.time_from < QDateTime::currentDateTime()) {
            QMessageBox::warning(this, "Ошибка", "Нельзя записать на прошедший слот.");
            return;
        }
        // Найти существующую запись для этого слота
        QList<Appointment> appts = dataManager.getAppointmentsByDoctor(doctorIdForBooking);
        for (const Appointment &a : appts) {
            if (a.id_ap_sch == scheduleIdUsed) {
                existingApptId = a.id_ap;
                existingAppt = a;
                break;
            }
        }
        QString st = sch.status.trimmed().toLower();
        // Разрешаем, если есть существующий приём (обновление), иначе блокируем занятые/завершённые
        if (existingApptId < 0 && (st == "booked" || st == "busy" || st == "done")) {
            QMessageBox::warning(this, "Слот занят", "Этот слот уже занят или завершён.");
            return;
        }
    }

    Appointment ap;
    bool isUpdate = false;
    if (existingApptId > 0) {
        ap = existingAppt;
        ap.id_patient = patientId;
        ap.completed = false;
        isUpdate = true;
    } else {
        ap.id_ap = dataManager.getNextAppointmentId();
        ap.id_doctor = doctorIdForBooking;
        ap.id_patient = patientId;
        ap.completed = false;
    }
    ap.date = appointmentTime;
    ap.id_ap_sch = scheduleIdUsed;  // Привязать встречу к расписанию

    if (isUpdate) {
        dataManager.updateAppointment(ap);
    } else {
        dataManager.addAppointment(ap);
    }
    
    // Установить статус "занято" для использованного слота расписания
    if (scheduleIdUsed > 0) {
        AppointmentSchedule sch = dataManager.getScheduleById(scheduleIdUsed);
        if (sch.id_ap_sch > 0) {
            sch.status = "booked";
            dataManager.updateSchedule(sch);
        }
    }
    
    QString msg = (mode == 0 ? "Пациент записан на прием" : "Пациент успешно записан");
    if (isUpdate) msg = "Запись обновлена";
    QMessageBox::information(this, "Готово", msg);
    emit appointmentSaved();
    
    accept();
}

void DoctorVisitDialog::onFinishVisit() {
    if (!patientCombo) {
        QMessageBox::warning(this, "Ошибка", "Виджет пациента не инициализирован");
        return;
    }
    
    int patientId = patientCombo->currentData().toInt();
    int appointmentIdToFinish = currentAppointmentId;
    
    // Если currentAppointmentId не установлен, найти незавершенный прием
    if (appointmentIdToFinish <= 0) {
        QList<Appointment> appts = dataManager.getPatientAppointments(patientId);
        for (const Appointment &a : appts) {
            if (a.id_doctor == doctorId && (!a.completed)) {
                appointmentIdToFinish = a.id_ap;
                break;
            }
        }
    }
    
    if (appointmentIdToFinish > 0) {
        Appointment ap = dataManager.getAppointmentById(appointmentIdToFinish);
        ap.completed = true;
        dataManager.updateAppointment(ap);
        
        // Save recipe (if diagnosis / complaints / recommendations provided)
        if (complaintsEdit && recommendationsEdit) {
            Recipe r;
            r.id = dataManager.getNextRecipeId();
            r.id_ap = ap.id_ap;
            int diagId = -1;
            if (diagnosisCombo) {
                QVariant v = diagnosisCombo->currentData();
                if (v.isValid()) diagId = v.toInt();
            }
            // If no diagnosis selected (diagId == -1) and there are diagnoses available, use first one as default
            if (diagId == -1 && diagnosisCombo && diagnosisCombo->count() > 0) {
                diagId = diagnosisCombo->itemData(0).toInt();
            }
            r.id_diagnosis = diagId;
            r.complaints = complaintsEdit->toPlainText();
            r.recommendations = recommendationsEdit->toPlainText();
            dataManager.addRecipe(r);
        }

        // После завершения приема помечаем слот как завершённый, но не удаляем
        if (ap.id_ap_sch > 0) {
            AppointmentSchedule sch = dataManager.getScheduleById(ap.id_ap_sch);
            if (sch.id_ap_sch > 0) {
                sch.status = "done";
                dataManager.updateSchedule(sch);
            }
        }
    }
    
    QMessageBox::information(this, "Готово", "Прием завершён");
    emit visitCompleted();
    accept();
}
void DoctorVisitDialog::onSelectDoctorForBooking() {
    loadSchedulesForSelectedDoctor();
}

void DoctorVisitDialog::onCancelBooking() {
    // Cancel booking for a selected schedule slot (only if mode == booking and scheduleId >= 0)
    if (mode != 1 || scheduleId < 0) {
        reject();
        return;
    }
    
    AppointmentSchedule sch = dataManager.getScheduleById(scheduleId);
    if (sch.id_ap_sch <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось найти слот расписания");
        return;
    }
    
    // Mark schedule as free (not booked)
    sch.status = "free";
    dataManager.updateSchedule(sch);
    
    // Delete any associated appointments for this schedule
    QList<Appointment> appts = dataManager.getAppointmentsByDoctor(doctorId);
    for (const Appointment &a : appts) {
        if (a.id_ap_sch == scheduleId) {
            dataManager.deleteAppointment(a.id_ap);
        }
    }
    
    QMessageBox::information(this, "Готово", "Запись отменена");
    emit appointmentSaved(); // Refresh parent view
    reject();
}

void DoctorVisitDialog::onRepeatBooking() {
    if (!patientCombo) return;
    int patientId = patientCombo->currentData().toInt();
    if (patientId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите пациента, чтобы записать повторно.");
        return;
    }

    // Менеджерский сценарий: выбрать любого врача и любое доступное окно
    DoctorVisitDialog bookingDlg(doctorId, -1, 1, this);
    bookingDlg.setBookingPatient(patientId, true); // фиксируем пациента
    if (bookingDlg.exec() == QDialog::Accepted) {
        emit appointmentSaved();
    }
}
