#include "admins/adminwidget.h"
#include "admins/statisticswidget.h"
#include "datamanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QMessageBox>
#include <QCoreApplication>
#include <QHeaderView>
#include <QScrollArea>
#include "admins/patientappointmentsviewer.h"
#include "managers/managerscheduleviewer.h"

// Implementation of AdminWidget
AdminWidget::AdminWidget(QWidget *parent)
    : QWidget(parent)
{
    QString dataPath = QCoreApplication::applicationDirPath() + "/../data";
    dataManager = new DataManager(dataPath);
    buildUI();

    loadDoctors();
    loadPatients();
    loadManagers();
}

void AdminWidget::setUser(const LoginUser &user) {
    currentUser = user;
}

void AdminWidget::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);
    
    tabs = new QTabWidget(this);

    // Doctors tab
    doctorsTab = new QWidget();
    QVBoxLayout *dlay = new QVBoxLayout(doctorsTab);
    doctorsSearchEdit = new QLineEdit();
    doctorsSearchEdit->setPlaceholderText("Поиск врачей по имени, email, ID...");
    dlay->addWidget(doctorsSearchEdit);
    doctorsTable = new QTableWidget();
    doctorsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // CHANGED: Make table read-only
    doctorsTable->setColumnCount(4);
    doctorsTable->setHorizontalHeaderLabels({"ID", "ФИО", "Email", "Специализация"});
    dlay->addWidget(doctorsTable);
    QHBoxLayout *dActions = new QHBoxLayout();
    addDoctorBtn = new QPushButton("Добавить");
    editDoctorBtn = new QPushButton("Изменить");
    deleteDoctorBtn = new QPushButton("Удалить");
    manageScheduleBtn = new QPushButton("Управлять расписанием");
    dActions->addWidget(addDoctorBtn);
    dActions->addWidget(editDoctorBtn);
    dActions->addWidget(deleteDoctorBtn);
    dActions->addWidget(manageScheduleBtn);
    dActions->addStretch();
    dlay->addLayout(dActions);
    tabs->addTab(doctorsTab, "Врачи");

    // Patients tab
    patientsTab = new QWidget();
    QVBoxLayout *play = new QVBoxLayout(patientsTab);
    patientsSearchEdit = new QLineEdit();
    patientsSearchEdit->setPlaceholderText("Поиск пациентов по имени, email, ID...");
    play->addWidget(patientsSearchEdit);
    patientsTable = new QTableWidget();
    patientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // CHANGED: Make table read-only
    patientsTable->setColumnCount(4);
    patientsTable->setHorizontalHeaderLabels({"ID", "ФИО", "Email", "Телефон"});
    play->addWidget(patientsTable);
    QHBoxLayout *pActions = new QHBoxLayout();
    addPatientBtn = new QPushButton("Добавить");
    editPatientBtn = new QPushButton("Изменить");
    deletePatientBtn = new QPushButton("Удалить");
    viewAppointmentsBtn = new QPushButton("Просмотреть приёмы");
    pActions->addWidget(addPatientBtn);
    pActions->addWidget(editPatientBtn);
    pActions->addWidget(deletePatientBtn);
    pActions->addWidget(viewAppointmentsBtn);
    pActions->addStretch();
    play->addLayout(pActions);
    tabs->addTab(patientsTab, "Пациенты");

    // Managers tab
    managersTab = new QWidget();
    QVBoxLayout *mlay = new QVBoxLayout(managersTab);
    managersSearchEdit = new QLineEdit();
    managersSearchEdit->setPlaceholderText("Поиск менеджеров по имени, email, ID...");
    mlay->addWidget(managersSearchEdit);
    managersTable = new QTableWidget();
    managersTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // CHANGED: Make table read-only
    managersTable->setColumnCount(3);
    managersTable->setHorizontalHeaderLabels({"ID", "ФИО", "Email"});
    mlay->addWidget(managersTable);
    QHBoxLayout *mActions = new QHBoxLayout();
    addManagerBtn = new QPushButton("Добавить");
    editManagerBtn = new QPushButton("Изменить");
    deleteManagerBtn = new QPushButton("Удалить");
    mActions->addWidget(addManagerBtn);
    mActions->addWidget(editManagerBtn);
    mActions->addWidget(deleteManagerBtn);
    mActions->addStretch();
    mlay->addLayout(mActions);
    tabs->addTab(managersTab, "Менеджеры");

    // Statistics tab
    statisticsTab = new QWidget();
    QVBoxLayout *sl = new QVBoxLayout(statisticsTab);
    statisticsWidget = new StatisticsWidget(dataManager, statisticsTab);
    // Wrap statistics widget in a scroll area so page can be scrolled when content is large
    QScrollArea *statScroll = new QScrollArea(statisticsTab);
    statScroll->setWidgetResizable(true);
    statScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    statScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    statScroll->setWidget(statisticsWidget);
    sl->addWidget(statScroll);
    tabs->addTab(statisticsTab, "Статистика");

    main->addWidget(tabs);

    // Connections
    connect(addDoctorBtn, &QPushButton::clicked, this, &AdminWidget::onAddDoctor);
    connect(editDoctorBtn, &QPushButton::clicked, this, &AdminWidget::onEditDoctor);
    connect(deleteDoctorBtn, &QPushButton::clicked, this, &AdminWidget::onDeleteDoctor);
    connect(manageScheduleBtn, &QPushButton::clicked, this, &AdminWidget::onManageDoctorSchedule);
    connect(doctorsSearchEdit, &QLineEdit::textChanged, this, &AdminWidget::onDoctorsFilterChanged);

    connect(addPatientBtn, &QPushButton::clicked, this, &AdminWidget::onAddPatient);
    connect(editPatientBtn, &QPushButton::clicked, this, &AdminWidget::onEditPatient);
    connect(deletePatientBtn, &QPushButton::clicked, this, &AdminWidget::onDeletePatient);
    connect(viewAppointmentsBtn, &QPushButton::clicked, this, &AdminWidget::onViewPatientAppointments);
    connect(patientsSearchEdit, &QLineEdit::textChanged, this, &AdminWidget::onPatientsFilterChanged);

    connect(addManagerBtn, &QPushButton::clicked, this, &AdminWidget::onAddManager);
    connect(editManagerBtn, &QPushButton::clicked, this, &AdminWidget::onEditManager);
    connect(deleteManagerBtn, &QPushButton::clicked, this, &AdminWidget::onDeleteManager);
    connect(managersSearchEdit, &QLineEdit::textChanged, this, &AdminWidget::onManagersFilterChanged);
    
    // Auto-refresh statistics on tab switch
    connect(tabs, QOverload<int>::of(&QTabWidget::currentChanged), this, [this](int idx) {
        if (tabs->widget(idx) == statisticsTab) {
            statisticsWidget->refresh();
        }
    });
}

void AdminWidget::loadDoctors() {
    doctorsTable->setRowCount(0);
    allDoctors = dataManager->getAllDoctors();
    doctorsTable->setRowCount(allDoctors.size());
    int r = 0;
    for (const Doctor &d : allDoctors) {
        doctorsTable->setItem(r, 0, new QTableWidgetItem(QString::number(d.id_doctor)));
        doctorsTable->setItem(r, 1, new QTableWidgetItem(d.fullName()));
        doctorsTable->setItem(r, 2, new QTableWidgetItem(d.email));
        Specialization s = dataManager->getSpecializationById(d.id_spec);
        doctorsTable->setItem(r, 3, new QTableWidgetItem(s.name));
        ++r;
    }
    // Scale columns to content: ID and text columns sized to contents, last column stretches if space remains
    doctorsTable->resizeColumnsToContents();
    doctorsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    doctorsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    doctorsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    doctorsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    // Clear search
    doctorsSearchEdit->blockSignals(true);
    doctorsSearchEdit->clear();
    doctorsSearchEdit->blockSignals(false);
}

void AdminWidget::onAddDoctor() {
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить врача");
    QFormLayout form(&dlg);
    QLineEdit *fname = new QLineEdit();
    QLineEdit *lname = new QLineEdit();
    QLineEdit *tname = new QLineEdit();
    QDateEdit *bdate = new QDateEdit();
    bdate->setCalendarPopup(true);
    bdate->setDisplayFormat("yyyy-MM-dd");
    bdate->setDate(QDate::currentDate());
    QLineEdit *email = new QLineEdit();
    QLineEdit *phone = new QLineEdit();
    QLineEdit *password = new QLineEdit(); password->setEchoMode(QLineEdit::Password);
    QComboBox *specCb = new QComboBox();
    QList<Specialization> specs = dataManager->getAllSpecializations();
    for (const Specialization &s : specs) specCb->addItem(s.name, s.id_spec);
    form.addRow("Имя:", fname);
    form.addRow("Фамилия:", lname);
    form.addRow("Отчество:", tname);
    form.addRow("Дата рождения:", bdate);
    form.addRow("Email:", email);
    form.addRow("Телефон:", phone);
    form.addRow("Пароль:", password);
    form.addRow("Специализация:", specCb);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    // Validation: required fields
    if (fname->text().trimmed().isEmpty() || lname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя и фамилия обязательны.");
        return;
    }
    QString em = email->text().trimmed();
    if (!em.isEmpty() && !em.contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный email.");
        return;
    }
    // Check duplicate email among doctors
    Doctor existingDoc = dataManager->getDoctorByEmail(em);
    if (!em.isEmpty() && existingDoc.id_doctor > 0) {
        QMessageBox::warning(this, "Ошибка", "Email уже используется другим врачом.");
        return;
    }
    QString pass = password->text();
    if (pass.length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов.");
        return;
    }

    Doctor d;
    d.id_doctor = dataManager->getNextDoctorId();
    d.fname = fname->text().trimmed();
    d.lname = lname->text().trimmed();
    d.tname = tname->text().trimmed();
    d.bdate = bdate->date();
    d.email = em;
    d.phone_number = phone->text().trimmed();
    d.id_spec = specCb->currentData().toInt();
    d.password = hashPassword(pass);
    dataManager->addDoctor(d);
    loadDoctors();
}

void AdminWidget::onEditDoctor() {
    QTableWidgetItem *it = doctorsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите врача"); return; }
    int row = it->row();
    int id = doctorsTable->item(row, 0)->text().toInt();
    Doctor d = dataManager->getDoctorById(id);
    QDialog dlg(this);
    dlg.setWindowTitle("Редактировать врача");
    QFormLayout form(&dlg);
    QLineEdit *fname = new QLineEdit(d.fname);
    QLineEdit *lname = new QLineEdit(d.lname);
    QLineEdit *tname = new QLineEdit(d.tname);
    QDateEdit *bdate = new QDateEdit(d.bdate.isValid() ? d.bdate : QDate::currentDate());
    bdate->setCalendarPopup(true);
    bdate->setDisplayFormat("yyyy-MM-dd");
    QLineEdit *email = new QLineEdit(d.email);
    QLineEdit *phone = new QLineEdit(d.phone_number);
    QLineEdit *password = new QLineEdit(); password->setEchoMode(QLineEdit::Password);
    QComboBox *specCb = new QComboBox();
    QList<Specialization> specs = dataManager->getAllSpecializations();
    int selIndex = 0;
    for (int i = 0; i < specs.size(); ++i) { specCb->addItem(specs[i].name, specs[i].id_spec); if (specs[i].id_spec == d.id_spec) selIndex = i; }
    specCb->setCurrentIndex(selIndex);
    form.addRow("Имя:", fname);
    form.addRow("Фамилия:", lname);
    form.addRow("Отчество:", tname);
    form.addRow("Дата рождения:", bdate);
    form.addRow("Email:", email);
    form.addRow("Телефон:", phone);
    form.addRow("Новый пароль (оставьте пустым, чтобы не менять):", password);
    form.addRow("Специализация:", specCb);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    // Validation
    if (fname->text().trimmed().isEmpty() || lname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя и фамилия обязательны.");
        return;
    }
    QString em = email->text().trimmed();
    if (!em.isEmpty() && !em.contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный email.");
        return;
    }
    // Check duplicate email (except current doctor)
    Doctor existingDoc = dataManager->getDoctorByEmail(em);
    if (!em.isEmpty() && existingDoc.id_doctor > 0 && existingDoc.id_doctor != d.id_doctor) {
        QMessageBox::warning(this, "Ошибка", "Email уже используется другим врачом.");
        return;
    }
    if (!password->text().isEmpty() && password->text().length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов.");
        return;
    }

    d.fname = fname->text().trimmed();
    d.lname = lname->text().trimmed();
    d.tname = tname->text().trimmed();
    d.bdate = bdate->date();
    d.email = em;
    d.phone_number = phone->text().trimmed();
    if (!password->text().isEmpty()) d.password = hashPassword(password->text());
    d.id_spec = specCb->currentData().toInt();
    dataManager->updateDoctor(d);
    loadDoctors();
}

void AdminWidget::onDeleteDoctor() {
    QTableWidgetItem *it = doctorsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите врача"); return; }
    int row = it->row();
    int id = doctorsTable->item(row, 0)->text().toInt();
    
    // Check for associated schedules and appointments
    QList<AppointmentSchedule> schedules = dataManager->getDoctorSchedules(id);
    QList<Appointment> appointments = dataManager->getAppointmentsByDoctor(id);
    
    QString msg = "Удалить врача?\n";
    if (!schedules.isEmpty() || !appointments.isEmpty()) {
        msg += "\nВнимание: Будут удалены ";
        if (!schedules.isEmpty()) {
            msg += QString("%1 слотов расписания").arg(schedules.size());
        }
        if (!appointments.isEmpty()) {
            if (!schedules.isEmpty()) msg += " и ";
            msg += QString("%1 приемов").arg(appointments.size());
        }
        msg += ".";
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтвердите", msg);
    if (reply != QMessageBox::Yes) return;
    
    // Delete associated schedules
    for (const auto& s : schedules) {
        dataManager->deleteSchedule(s.id_ap_sch);
    }
    
    // Delete associated appointments
    for (const auto& a : appointments) {
        dataManager->deleteAppointment(a.id_ap);
    }
    
    dataManager->deleteDoctor(id);
    loadDoctors();
}

void AdminWidget::onManageDoctorSchedule() {
    QTableWidgetItem *it = doctorsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите врача"); return; }
    int row = it->row();
    int id = doctorsTable->item(row, 0)->text().toInt();
    // Open the ManagerScheduleViewer as a dialog-like popup and preselect the chosen doctor
    ManagerScheduleViewer *viewer = new ManagerScheduleViewer(dataManager, this);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setWindowFlag(Qt::Dialog);
    viewer->setWindowModality(Qt::WindowModal);
    viewer->setWindowTitle("Управление расписанием");
    viewer->resize(900, 600);
    viewer->setCurrentDoctor(id);
    viewer->show();
}

void AdminWidget::loadPatients() {
    patientsTable->setRowCount(0);
    allPatients = dataManager->getAllPatients();
    patientsTable->setRowCount(allPatients.size());
    int r = 0;
    for (const Patient &p : allPatients) {
        patientsTable->setItem(r, 0, new QTableWidgetItem(QString::number(p.id_patient)));
        patientsTable->setItem(r, 1, new QTableWidgetItem(p.fullName()));
        patientsTable->setItem(r, 2, new QTableWidgetItem(p.email));
        patientsTable->setItem(r, 3, new QTableWidgetItem(p.phone_number));
        ++r;
    }
    patientsTable->resizeColumnsToContents();
    patientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    patientsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    patientsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    patientsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    // Clear search
    patientsSearchEdit->blockSignals(true);
    patientsSearchEdit->clear();
    patientsSearchEdit->blockSignals(false);
}

void AdminWidget::onAddPatient() {
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить пациента");
    QFormLayout form(&dlg);
    QLineEdit *fname = new QLineEdit();
    QLineEdit *lname = new QLineEdit();
    QLineEdit *tname = new QLineEdit();
    QDateEdit *bdate = new QDateEdit();
    bdate->setCalendarPopup(true);
    bdate->setDisplayFormat("yyyy-MM-dd");
    bdate->setDate(QDate::currentDate());
    QLineEdit *email = new QLineEdit();
    QLineEdit *phone = new QLineEdit();
    QLineEdit *snils = new QLineEdit();
    QLineEdit *oms = new QLineEdit();
    QLineEdit *password = new QLineEdit(); password->setEchoMode(QLineEdit::Password);
    form.addRow("Имя:", fname);
    form.addRow("Фамилия:", lname);
    form.addRow("Отчество:", tname);
    form.addRow("Дата рождения:", bdate);
    form.addRow("Email:", email);
    form.addRow("Телефон:", phone);
    form.addRow("SNILS:", snils);
    form.addRow("OMS:", oms);
    form.addRow("Пароль:", password);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    
    // Validation
    if (fname->text().trimmed().isEmpty() || lname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя и фамилия обязательны.");
        return;
    }
    QString em = email->text().trimmed();
    if (!em.isEmpty() && !em.contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный email.");
        return;
    }
    if (!em.isEmpty() && dataManager->emailExists(em)) {
        QMessageBox::warning(this, "Ошибка", "Email уже используется.");
        return;
    }
    QString pass = password->text();
    if (pass.length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов.");
        return;
    }
    
    Patient p;
    p.id_patient = dataManager->getNextPatientId();
    p.fname = fname->text().trimmed();
    p.lname = lname->text().trimmed();
    p.tname = tname->text().trimmed();
    p.bdate = bdate->date().toString("yyyy-MM-dd");
    p.email = em;
    p.phone_number = phone->text().trimmed();
    p.snils = snils->text().trimmed();
    p.oms = oms->text().trimmed();
    p.password = hashPassword(pass);
    dataManager->addPatient(p);
    loadPatients();
}

void AdminWidget::onEditPatient() {
    QTableWidgetItem *it = patientsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите пациента"); return; }
    int row = it->row();
    int id = patientsTable->item(row, 0)->text().toInt();
    Patient p = dataManager->getPatientById(id);
    QDialog dlg(this);
    dlg.setWindowTitle("Редактировать пациента");
    QFormLayout form(&dlg);
    QLineEdit *fname = new QLineEdit(p.fname);
    QLineEdit *lname = new QLineEdit(p.lname);
    QLineEdit *tname = new QLineEdit(p.tname);
    QDateEdit *bdate = new QDateEdit(QDate::fromString(p.bdate, "yyyy-MM-dd"));
    bdate->setCalendarPopup(true);
    bdate->setDisplayFormat("yyyy-MM-dd");
    QLineEdit *email = new QLineEdit(p.email);
    QLineEdit *phone = new QLineEdit(p.phone_number);
    QLineEdit *snils = new QLineEdit(p.snils);
    QLineEdit *oms = new QLineEdit(p.oms);
    QLineEdit *password = new QLineEdit(); password->setEchoMode(QLineEdit::Password);
    form.addRow("Имя:", fname);
    form.addRow("Фамилия:", lname);
    form.addRow("Отчество:", tname);
    form.addRow("Дата рождения:", bdate);
    form.addRow("Email:", email);
    form.addRow("Телефон:", phone);
    form.addRow("SNILS:", snils);
    form.addRow("OMS:", oms);
    form.addRow("Новый пароль (оставьте пустым, чтобы не менять):", password);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    
    // Validation
    if (fname->text().trimmed().isEmpty() || lname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя и фамилия обязательны.");
        return;
    }
    QString em = email->text().trimmed();
    if (!em.isEmpty() && !em.contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный email.");
        return;
    }
    // Check duplicate email (except self)
    Patient existingPat = dataManager->getPatientByEmail(em);
    if (!em.isEmpty() && existingPat.id_patient > 0 && existingPat.id_patient != p.id_patient) {
        QMessageBox::warning(this, "Ошибка", "Email уже используется.");
        return;
    }
    if (!password->text().isEmpty() && password->text().length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов.");
        return;
    }
    
    p.fname = fname->text().trimmed();
    p.lname = lname->text().trimmed();
    p.tname = tname->text().trimmed();
    p.bdate = bdate->date().toString("yyyy-MM-dd");
    p.email = em;
    p.phone_number = phone->text().trimmed();
    p.snils = snils->text().trimmed();
    p.oms = oms->text().trimmed();
    if (!password->text().isEmpty()) p.password = hashPassword(password->text());
    dataManager->updatePatient(p);
    loadPatients();
}

void AdminWidget::onDeletePatient() {
    QTableWidgetItem *it = patientsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите пациента"); return; }
    int row = it->row();
    int id = patientsTable->item(row, 0)->text().toInt();
    
    // Check for associated appointments
    QList<Appointment> appointments = dataManager->getPatientAppointments(id);
    QString msg = "Удалить пациента?\n";
    if (!appointments.isEmpty()) {
        msg += QString("\nВнимание: Будут удалены %1 приемов.").arg(appointments.size());
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтвердите", msg);
    if (reply != QMessageBox::Yes) return;
    dataManager->deletePatient(id);
    loadPatients();
}

void AdminWidget::onViewPatientAppointments() {
    QTableWidgetItem *it = patientsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите пациента"); return; }
    int row = it->row();
    int id = patientsTable->item(row, 0)->text().toInt();
    // Open patient appointments viewer as a popup dialog
    PatientAppointmentsViewer *viewer = new PatientAppointmentsViewer(dataManager, this);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setWindowFlag(Qt::Dialog);
    viewer->setWindowModality(Qt::WindowModal);
    viewer->setWindowTitle("Приёмы пациента");
    viewer->resize(800, 500);
    viewer->setCurrentPatient(id);
    viewer->show();
}

void AdminWidget::loadManagers() {
    managersTable->setRowCount(0);
    allManagers = dataManager->getAllManagers();
    managersTable->setRowCount(allManagers.size());
    int r = 0;
    for (const Manager &m : allManagers) {
        managersTable->setItem(r, 0, new QTableWidgetItem(QString::number(m.id)));
        managersTable->setItem(r, 1, new QTableWidgetItem(m.fullName()));
        managersTable->setItem(r, 2, new QTableWidgetItem(m.email));
        ++r;
    }
    managersTable->resizeColumnsToContents();
    managersTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    managersTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    managersTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    // Clear search
    managersSearchEdit->blockSignals(true);
    managersSearchEdit->clear();
    managersSearchEdit->blockSignals(false);
}

void AdminWidget::onAddManager() {
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить менеджера");
    QFormLayout form(&dlg);
    QLineEdit *fname = new QLineEdit();
    QLineEdit *lname = new QLineEdit();
    QLineEdit *email = new QLineEdit();
    QLineEdit *password = new QLineEdit(); password->setEchoMode(QLineEdit::Password);
    form.addRow("Имя:", fname);
    form.addRow("Фамилия:", lname);
    form.addRow("Email:", email);
    form.addRow("Пароль:", password);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    
    // Validation
    if (fname->text().trimmed().isEmpty() || lname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя и фамилия обязательны.");
        return;
    }
    QString em = email->text().trimmed();
    if (!em.isEmpty() && !em.contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный email.");
        return;
    }
    Manager existingMgr = dataManager->getManagerByEmail(em);
    if (!em.isEmpty() && existingMgr.id > 0) {
        QMessageBox::warning(this, "Ошибка", "Email уже используется.");
        return;
    }
    QString pass = password->text();
    if (pass.length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов.");
        return;
    }
    
    Manager m;
    m.id = dataManager->getNextManagerId();
    m.fname = fname->text().trimmed();
    m.lname = lname->text().trimmed();
    m.email = em;
    m.password = hashPassword(pass);
    dataManager->addManager(m);
    loadManagers();
}

void AdminWidget::onEditManager() {
    QTableWidgetItem *it = managersTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите менеджера"); return; }
    int row = it->row();
    int id = managersTable->item(row, 0)->text().toInt();
    Manager m = dataManager->getManagerById(id);
    QDialog dlg(this);
    dlg.setWindowTitle("Редактировать менеджера");
    QFormLayout form(&dlg);
    QLineEdit *fname = new QLineEdit(m.fname);
    QLineEdit *lname = new QLineEdit(m.lname);
    QLineEdit *email = new QLineEdit(m.email);
    QLineEdit *password = new QLineEdit(); password->setEchoMode(QLineEdit::Password);
    form.addRow("Имя:", fname);
    form.addRow("Фамилия:", lname);
    form.addRow("Email:", email);
    form.addRow("Новый пароль (оставьте пустым, чтобы не менять):", password);
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttonBox);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    
    // Validation
    if (fname->text().trimmed().isEmpty() || lname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Имя и фамилия обязательны.");
        return;
    }
    QString em = email->text().trimmed();
    if (!em.isEmpty() && !em.contains('@')) {
        QMessageBox::warning(this, "Ошибка", "Неверный email.");
        return;
    }
    // Check duplicate email (except self)
    Manager existingMgr = dataManager->getManagerByEmail(em);
    if (!em.isEmpty() && existingMgr.id > 0 && existingMgr.id != m.id) {
        QMessageBox::warning(this, "Ошибка", "Email уже используется.");
        return;
    }
    if (!password->text().isEmpty() && password->text().length() < 6) {
        QMessageBox::warning(this, "Ошибка", "Пароль должен быть не менее 6 символов.");
        return;
    }
    
    m.fname = fname->text().trimmed();
    m.lname = lname->text().trimmed();
    m.email = em;
    if (!password->text().isEmpty()) m.password = hashPassword(password->text());
    dataManager->updateManager(m);
    loadManagers();
}

void AdminWidget::onDeleteManager() {
    QTableWidgetItem *it = managersTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите менеджера"); return; }
    int row = it->row();
    int id = managersTable->item(row, 0)->text().toInt();
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтвердите", "Удалить менеджера?");
    if (reply != QMessageBox::Yes) return;
    dataManager->deleteManager(id);
    loadManagers();
}

void AdminWidget::onDoctorsFilterChanged(const QString &text) {
    for (int row = 0; row < doctorsTable->rowCount(); ++row) {
        bool match = false;
        if (text.isEmpty()) {
            match = true;
        } else {
            // Search in ID, Full Name, Email, Specialization
            for (int col = 0; col < doctorsTable->columnCount(); ++col) {
                QTableWidgetItem *item = doctorsTable->item(row, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        doctorsTable->setRowHidden(row, !match);
    }
}

void AdminWidget::onPatientsFilterChanged(const QString &text) {
    for (int row = 0; row < patientsTable->rowCount(); ++row) {
        bool match = false;
        if (text.isEmpty()) {
            match = true;
        } else {
            // Search in ID, Full Name, Email, Phone
            for (int col = 0; col < patientsTable->columnCount(); ++col) {
                QTableWidgetItem *item = patientsTable->item(row, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        patientsTable->setRowHidden(row, !match);
    }
}

void AdminWidget::onManagersFilterChanged(const QString &text) {
    for (int row = 0; row < managersTable->rowCount(); ++row) {
        bool match = false;
        if (text.isEmpty()) {
            match = true;
        } else {
            // Search in ID, Full Name, Email
            for (int col = 0; col < managersTable->columnCount(); ++col) {
                QTableWidgetItem *item = managersTable->item(row, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                    match = true;
                    break;
                }
            }
        }
        managersTable->setRowHidden(row, !match);
    }
}

#include "admins/adminwidget.moc"
