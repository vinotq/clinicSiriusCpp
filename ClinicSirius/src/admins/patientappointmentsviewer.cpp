#include "admins/patientappointmentsviewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QDateTime>
#include "../patients/appointmentbookingwidget.h"
#include "../patients/createpatientdialog.h" // CHANGED: Include for patient edit dialog
#include <QPushButton> // CHANGED: For add button

// CHANGED: For slot declaration
void PatientAppointmentsViewer::onAddAppointmentClicked() {
    if (m_currentPatientId <= 0) return;
    AppointmentBookingWidget *booking = new AppointmentBookingWidget();
    booking->setAttribute(Qt::WA_DeleteOnClose);
    LoginUser adminUser(LoginUser::ADMIN, -1, "Администратор");
    booking->setUser(adminUser);
    // CHANGED: Use setInitialSelection for admin context, patient flows are handled inside selection dialog
    booking->setInitialSelection(-1);
    booking->setWindowTitle("Создать приём");
    booking->resize(900,700);
    booking->show();
    connect(booking, &QObject::destroyed, this, [this](QObject*){ loadAppointmentsForPatient(m_currentPatientId); });
}

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QDateTime>
#include "../patients/appointmentbookingwidget.h"
#include "../patients/createpatientdialog.h" // CHANGED: Include for patient edit dialog
#include <QPushButton> // CHANGED: For add button

PatientAppointmentsViewer::PatientAppointmentsViewer(DataManager *dm, QWidget *parent)
    : QWidget(parent), m_dm(dm) {
    buildUI();
}

void PatientAppointmentsViewer::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);
    m_header = new QLabel("Приёмы пациента");
    main->addWidget(m_header);

    // CHANGED: Add create appointment button
    QPushButton *addBtn = new QPushButton("📅 Создать приём");
    main->addWidget(addBtn);
    connect(addBtn, &QPushButton::clicked, this, &PatientAppointmentsViewer::onAddAppointmentClicked);


    m_filterEdit = new QLineEdit();
    m_filterEdit->setPlaceholderText("Фильтр по врачу/дате/статусу...");
    main->addWidget(m_filterEdit);

    m_table = new QTableWidget();
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers); // CHANGED: Make table read-only
    // Columns: ID, DateTime, Doctor, Specialization, Room
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"ID", "Дата и время", "Врач", "Специальность", "Кабинет"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    main->addWidget(m_table, 1);

    connect(m_table, &QTableWidget::customContextMenuRequested, this, &PatientAppointmentsViewer::onTableContextMenu);
    connect(m_filterEdit, &QLineEdit::textChanged, this, [this](const QString &txt){
        for (int r = 0; r < m_table->rowCount(); ++r) {
            bool match = txt.isEmpty();
            for (int c = 0; c < m_table->columnCount(); ++c) {
                QTableWidgetItem *it = m_table->item(r, c);
                if (it && it->text().contains(txt, Qt::CaseInsensitive)) { match = true; break; }
            }
            m_table->setRowHidden(r, !match);
        }
    });
}

void PatientAppointmentsViewer::setCurrentPatient(int patientId) {
    m_currentPatientId = patientId;
    loadAppointmentsForPatient(patientId);
}

void PatientAppointmentsViewer::loadAppointmentsForPatient(int patientId) {
    m_table->clearContents();
    QList<Appointment> list = m_dm->getPatientAppointments(patientId);
    m_table->setRowCount(list.size());
    int r = 0;
    for (const Appointment &a : list) {
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(a.id_ap));
        idItem->setData(Qt::UserRole, a.id_ap);
        m_table->setItem(r, 0, idItem);

        QString dt = a.date.isValid() ? a.date.toString("yyyy-MM-dd HH:mm") : QString();
        m_table->setItem(r, 1, new QTableWidgetItem(dt));

        Doctor doc = m_dm->getDoctorById(a.id_doctor);
        QString docName = doc.fullName();
        m_table->setItem(r, 2, new QTableWidgetItem(docName));
        QString specName;
        if (doc.id_spec > 0) {
            Specialization spec = m_dm->getSpecializationById(doc.id_spec);
            specName = spec.name;
        }
        m_table->setItem(r, 3, new QTableWidgetItem(specName));

        // resolve room via schedule if available
        QString roomStr;
        if (a.id_ap_sch > 0) {
            AppointmentSchedule sch = m_dm->getScheduleById(a.id_ap_sch);
            Room room = m_dm->getRoomById(sch.id_room);
            roomStr = room.room_number;
        }
        m_table->setItem(r, 4, new QTableWidgetItem(roomStr));

        ++r;
    }
}

void PatientAppointmentsViewer::onTableContextMenu(const QPoint &pos) {
    QTableWidgetItem *it = m_table->itemAt(pos);
    if (!it) return;
    int row = it->row();
    int apId = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    if (apId <= 0) return;

    // find appointment record
    Appointment target;
    bool found = false;
    for (const Appointment &a : m_dm->getAllAppointments()) {
        if (a.id_ap == apId) { target = a; found = true; break; }
    }
    if (!found) return;

    QMenu menu;
    QAction *reschedule = menu.addAction("🕒 Перенести приём");
    QAction *editPatient = menu.addAction("✍️ Редактировать пациента");
    QAction *cancel = menu.addAction("❌ Отменить запись");
    QAction *selected = menu.exec(m_table->viewport()->mapToGlobal(pos));
    if (!selected) return;

    if (selected == cancel) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтвердите", "Отменить эту запись?");
        if (reply != QMessageBox::Yes) return;
        m_dm->deleteAppointment(apId);
        loadAppointmentsForPatient(m_currentPatientId);
    } else if (selected == editPatient) { // CHANGED: Edit patient action
        Patient p = m_dm->getPatientById(target.id_patient);
        if (p.id_patient > 0) {
            CreatePatientDialog dlg(this, &p);
            if (dlg.exec() == QDialog::Accepted) {
                Patient np = dlg.getCreatedPatient();
                m_dm->updatePatient(np);
                loadAppointmentsForPatient(m_currentPatientId);
            }
        }
    } else if (selected == reschedule) {
        // find schedule id for this appointment
        AppointmentSchedule sch = m_dm->getScheduleById(target.id_ap_sch);
        AppointmentBookingWidget *booking = new AppointmentBookingWidget();
        booking->setAttribute(Qt::WA_DeleteOnClose);
        LoginUser managerUser(LoginUser::MANAGER, -1, "Менеджер");
        booking->setUser(managerUser);
        booking->setRescheduleMode(target.id_ap, sch.id_ap_sch);
        booking->setWindowTitle("Перенос приёма");
        booking->resize(900,700);
        booking->show();
        connect(booking, &QObject::destroyed, this, [this](QObject*){ loadAppointmentsForPatient(m_currentPatientId); });
    }
}
