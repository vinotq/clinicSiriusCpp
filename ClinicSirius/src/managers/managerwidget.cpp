#include "managers/managerwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include "patients/appointmentbookingwidget.h"
#include "managers/managerscheduleviewer.h"
#include "managers/patientmanagementdialog.h"
#include "managers/bulkoperationsdialog.h"

ManagerWidget::ManagerWidget(QWidget* parent)
    : QWidget(parent), m_dataManager(QString()) {
    QVBoxLayout* main = new QVBoxLayout(this);
    QHBoxLayout* header = new QHBoxLayout();

    QLabel* title = new QLabel("Панель менеджера");
    title->setStyleSheet("font-weight: bold; font-size: 18px;");
    header->addWidget(title);
    header->addStretch();

    main->addLayout(header);

    // Clinic management buttons
    QHBoxLayout* clinicBtns = new QHBoxLayout();
    m_viewSchedulesBtn = new QPushButton("📅 Управление расписанием");
    m_managePatientsBtn = new QPushButton("👥 Управление пациентами");
    m_bulkOpsBtn = new QPushButton("🛠 Массовые операции");
    clinicBtns->addWidget(m_viewSchedulesBtn);
    clinicBtns->addWidget(m_managePatientsBtn);
    clinicBtns->addWidget(m_bulkOpsBtn);
    clinicBtns->addStretch();
    main->addLayout(clinicBtns);

    m_appointmentsList = new QListWidget();
    main->addWidget(m_appointmentsList, 1);

    QHBoxLayout* btns = new QHBoxLayout();
    m_addBtn = new QPushButton("➕ Записать");
    m_cancelBtn = new QPushButton("✖ Отменить запись");
    m_rescheduleBtn = new QPushButton("🔁 Перенести");
    btns->addWidget(m_addBtn);
    btns->addWidget(m_rescheduleBtn);
    btns->addWidget(m_cancelBtn);
    btns->addStretch();
    main->addLayout(btns);

    connect(m_addBtn, &QPushButton::clicked, this, &ManagerWidget::onAddAppointment);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ManagerWidget::onCancelAppointment);
    connect(m_rescheduleBtn, &QPushButton::clicked, this, &ManagerWidget::onRescheduleAppointment);

    connect(m_viewSchedulesBtn, &QPushButton::clicked, this, &ManagerWidget::onViewClinicSchedules);
    connect(m_managePatientsBtn, &QPushButton::clicked, this, &ManagerWidget::onManagePatients);
    connect(m_bulkOpsBtn, &QPushButton::clicked, this, &ManagerWidget::onBulkOperations);

    refreshAppointments();
}

void ManagerWidget::setUser(const LoginUser &user) {
    m_user = user;
    refreshAppointments();
}

void ManagerWidget::refreshAppointments() {
    m_appointmentsList->clear();
    QList<Appointment> appts = m_dataManager.getAllAppointments();
    for (const Appointment &a : appts) {
        Patient p = m_dataManager.getPatientById(a.id_patient);
        Doctor d = m_dataManager.getDoctorById(a.id_doctor);
        QString text = QString("#%1 — %2 — %3 — %4")
            .arg(a.id_ap)
            .arg(p.fullName().isEmpty() ? QString("Пациент %1").arg(a.id_patient) : p.fullName())
            .arg(d.fullName().isEmpty() ? QString("Врач %1").arg(a.id_doctor) : d.fullName())
            .arg(a.date.isValid() ? a.date.toString("dd.MM.yyyy HH:mm") : QString("—"));

        QListWidgetItem* it = new QListWidgetItem(text);
        it->setData(Qt::UserRole, a.id_ap);
        m_appointmentsList->addItem(it);
    }
}

void ManagerWidget::onAddAppointment() {
    // Use existing AppointmentBookingWidget in a dialog so manager can create appointment
    QDialog dlg(this);
    dlg.setWindowTitle("Запись — менеджер");
    QVBoxLayout* l = new QVBoxLayout(&dlg);
    AppointmentBookingWidget* booking = new AppointmentBookingWidget(&dlg);
    LoginUser managerUser(LoginUser::MANAGER, m_user.id, m_user.name);
    booking->setUser(managerUser);
    l->addWidget(booking);
    dlg.resize(800, 600);
    dlg.exec();

    // After dialog closed, refresh list (booking widget writes directly to data)
    refreshAppointments();
}

void ManagerWidget::onCancelAppointment() {
    QListWidgetItem* it = m_appointmentsList->currentItem();
    if (!it) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись для отмены");
        return;
    }
    int id = it->data(Qt::UserRole).toInt();

    Appointment ap = m_dataManager.getAppointmentById(id);
    if (ap.id_ap <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось найти выбранную запись");
        return;
    }

    if (QMessageBox::question(this, "Подтвердить", QString("Отменить запись #%1?").arg(id)) != QMessageBox::Yes) {
        return;
    }

    // If appointment linked to a schedule, free it
    if (ap.id_ap_sch > 0) {
        AppointmentSchedule sch = m_dataManager.getScheduleById(ap.id_ap_sch);
        if (sch.id_ap_sch > 0) {
            sch.status = "free";
            m_dataManager.updateSchedule(sch);
        }
    }

    m_dataManager.deleteAppointment(id);
    QMessageBox::information(this, "Готово", "Запись отменена");
    refreshAppointments();
}

void ManagerWidget::onRescheduleAppointment() {
    QListWidgetItem* it = m_appointmentsList->currentItem();
    if (!it) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись для переноса");
        return;
    }
    int id = it->data(Qt::UserRole).toInt();
    Appointment ap = m_dataManager.getAppointmentById(id);
    if (ap.id_ap <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось найти выбранную запись");
        return;
    }

    // Load available schedules for the doctor
    QList<AppointmentSchedule> avail = m_dataManager.getAvailableSchedules(ap.id_doctor);
    if (avail.isEmpty()) {
        QMessageBox::information(this, "Нет слотов", "Нет доступных слотов для данного врача");
        return;
    }

    QStringList items;
    for (const AppointmentSchedule &s : avail) {
        items << QString("%1 — %2").arg(s.id_ap_sch).arg(s.time_from.toString("dd.MM.yyyy HH:mm"));
    }

    bool ok = false;
    QString choice = QInputDialog::getItem(this, "Выберите новый слот", "Доступные слоты:", items, 0, false, &ok);
    if (!ok || choice.isEmpty()) return;

    // Extract schedule id from chosen string (format: id — date)
    int newSchId = choice.split(" — ").first().toInt();
    if (newSchId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Некорректный слот");
        return;
    }

    // Free old schedule if exists
    if (ap.id_ap_sch > 0) {
        AppointmentSchedule oldSch = m_dataManager.getScheduleById(ap.id_ap_sch);
        if (oldSch.id_ap_sch > 0) {
            oldSch.status = "free";
            m_dataManager.updateSchedule(oldSch);
        }
    }

    // Book new schedule
    AppointmentSchedule newSch = m_dataManager.getScheduleById(newSchId);
    if (newSch.id_ap_sch <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить выбранный слот");
        return;
    }
    newSch.status = "booked";
    m_dataManager.updateSchedule(newSch);

    // Update appointment
    ap.id_ap_sch = newSch.id_ap_sch;
    ap.date = newSch.time_from;
    ap.id_doctor = newSch.id_doctor;
    m_dataManager.updateAppointment(ap);

    QMessageBox::information(this, "Готово", "Запись успешно перенесена");
    refreshAppointments();
}

void ManagerWidget::onViewClinicSchedules() {
    ManagerScheduleViewer dlg(this);
    // Allow manager to click and create bookings from the schedule viewer
    dlg.exec();
}

void ManagerWidget::onManagePatients() {
    PatientManagementDialog dlg(this);
    dlg.exec();
    // refresh appointments/patients lists after possible changes
    refreshAppointments();
}

void ManagerWidget::onBulkOperations() {
    BulkOperationsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        QMessageBox::information(this, "Готово", "Массовые операции применены");
        refreshAppointments();
    }
}
