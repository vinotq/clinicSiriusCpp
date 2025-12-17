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
#include <QIcon>
#include <QGroupBox>
#include <QLabel>
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
    loadSpecializations();
    loadRooms();
    loadDiagnoses();
}

void AdminWidget::setUser(const LoginUser &user) {
    currentUser = user;
}

void AdminWidget::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(12, 12, 12, 12);
    main->setSpacing(14);
    
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
    addDoctorBtn->setIcon(QIcon(":/images/icon-add.svg"));
    addDoctorBtn->setIconSize(QSize(16,16));
    editDoctorBtn = new QPushButton("Изменить");
    editDoctorBtn->setIcon(QIcon(":/images/icon-edit.svg"));
    editDoctorBtn->setIconSize(QSize(16,16));
    deleteDoctorBtn = new QPushButton("Удалить");
    deleteDoctorBtn->setIcon(QIcon(":/images/icon-trash.svg"));
    deleteDoctorBtn->setIconSize(QSize(16,16));
    manageScheduleBtn = new QPushButton("Управлять расписанием");
    manageScheduleBtn->setIcon(QIcon(":/images/icon-calendar.svg"));
    manageScheduleBtn->setIconSize(QSize(16,16));
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
    addPatientBtn->setIcon(QIcon(":/images/icon-add.svg"));
    addPatientBtn->setIconSize(QSize(16,16));
    editPatientBtn = new QPushButton("Изменить");
    editPatientBtn->setIcon(QIcon(":/images/icon-edit.svg"));
    editPatientBtn->setIconSize(QSize(16,16));
    deletePatientBtn = new QPushButton("Удалить");
    deletePatientBtn->setIcon(QIcon(":/images/icon-trash.svg"));
    deletePatientBtn->setIconSize(QSize(16,16));
    viewAppointmentsBtn = new QPushButton("Просмотреть приёмы");
    viewAppointmentsBtn->setIcon(QIcon(":/images/icon-upcoming.svg"));
    viewAppointmentsBtn->setIconSize(QSize(16,16));
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
    addManagerBtn->setIcon(QIcon(":/images/icon-add.svg"));
    addManagerBtn->setIconSize(QSize(16,16));
    editManagerBtn = new QPushButton("Изменить");
    editManagerBtn->setIcon(QIcon(":/images/icon-edit.svg"));
    editManagerBtn->setIconSize(QSize(16,16));
    deleteManagerBtn = new QPushButton("Удалить");
    deleteManagerBtn->setIcon(QIcon(":/images/icon-trash.svg"));
    deleteManagerBtn->setIconSize(QSize(16,16));
    mActions->addWidget(addManagerBtn);
    mActions->addWidget(editManagerBtn);
    mActions->addWidget(deleteManagerBtn);
    mActions->addStretch();
    mlay->addLayout(mActions);
    tabs->addTab(managersTab, "Менеджеры");

    // Directories tab (rooms, diagnoses, specializations)
    directoriesTab = new QWidget();
    QVBoxLayout *dirLayout = new QVBoxLayout(directoriesTab);
    dirLayout->setContentsMargins(10, 8, 10, 8);
    dirLayout->setSpacing(14);

    // Specializations section
    QGroupBox *specBox = new QGroupBox("Специализации");
    specBox->setStyleSheet("QGroupBox { margin-top: 6px; padding-top: 14px; }"
                           "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }");
    QVBoxLayout *specLayout = new QVBoxLayout(specBox);
    specLayout->setContentsMargins(10, 8, 10, 8);
    specLayout->setSpacing(10);
    specsSearchEdit = new QLineEdit();
    specsSearchEdit->setPlaceholderText("Поиск специализаций...");
    specsSearchEdit->addAction(QIcon(":/images/icon-specialization.svg"), QLineEdit::LeadingPosition);
    specLayout->addWidget(specsSearchEdit);
    specsTable = new QTableWidget();
    specsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    specsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    specsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    specsTable->setColumnCount(2);
    specsTable->setHorizontalHeaderLabels({"ID", "Название"});
    specLayout->addWidget(specsTable);
    QHBoxLayout *specActions = new QHBoxLayout();
    addSpecBtn = new QPushButton("Добавить");
    addSpecBtn->setIcon(QIcon(":/images/icon-add.svg"));
    addSpecBtn->setIconSize(QSize(16,16));
    editSpecBtn = new QPushButton("Изменить");
    editSpecBtn->setIcon(QIcon(":/images/icon-edit.svg"));
    editSpecBtn->setIconSize(QSize(16,16));
    deleteSpecBtn = new QPushButton("Удалить");
    deleteSpecBtn->setIcon(QIcon(":/images/icon-trash.svg"));
    deleteSpecBtn->setIconSize(QSize(16,16));
    specActions->addWidget(addSpecBtn);
    specActions->addWidget(editSpecBtn);
    specActions->addWidget(deleteSpecBtn);
    specActions->addStretch();
    specLayout->addLayout(specActions);
    dirLayout->addWidget(specBox);
    dirLayout->addSpacing(8);

    // Rooms section
    QGroupBox *roomBox = new QGroupBox("Кабинеты");
    roomBox->setStyleSheet("QGroupBox { margin-top: 6px; padding-top: 14px; }"
                           "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }");
    QVBoxLayout *roomLayout = new QVBoxLayout(roomBox);
    roomLayout->setContentsMargins(10, 8, 10, 8);
    roomLayout->setSpacing(10);
    roomsSearchEdit = new QLineEdit();
    roomsSearchEdit->setPlaceholderText("Поиск кабинетов...");
    roomsSearchEdit->addAction(QIcon(":/images/icon-room.svg"), QLineEdit::LeadingPosition);
    roomLayout->addWidget(roomsSearchEdit);
    roomsTable = new QTableWidget();
    roomsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    roomsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    roomsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    roomsTable->setColumnCount(2);
    roomsTable->setHorizontalHeaderLabels({"ID", "Номер"});
    roomLayout->addWidget(roomsTable);
    QHBoxLayout *roomActions = new QHBoxLayout();
    addRoomBtn = new QPushButton("Добавить");
    addRoomBtn->setIcon(QIcon(":/images/icon-add.svg"));
    addRoomBtn->setIconSize(QSize(16,16));
    editRoomBtn = new QPushButton("Изменить");
    editRoomBtn->setIcon(QIcon(":/images/icon-edit.svg"));
    editRoomBtn->setIconSize(QSize(16,16));
    deleteRoomBtn = new QPushButton("Удалить");
    deleteRoomBtn->setIcon(QIcon(":/images/icon-trash.svg"));
    deleteRoomBtn->setIconSize(QSize(16,16));
    roomActions->addWidget(addRoomBtn);
    roomActions->addWidget(editRoomBtn);
    roomActions->addWidget(deleteRoomBtn);
    roomActions->addStretch();
    roomLayout->addLayout(roomActions);
    dirLayout->addWidget(roomBox);
    dirLayout->addSpacing(8);

    // Diagnoses section
    QGroupBox *diagBox = new QGroupBox("Диагнозы");
    diagBox->setStyleSheet("QGroupBox { margin-top: 6px; padding-top: 14px; }"
                           "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px; }");
    QVBoxLayout *diagLayout = new QVBoxLayout(diagBox);
    diagLayout->setContentsMargins(10, 8, 10, 8);
    diagLayout->setSpacing(10);
    diagSearchEdit = new QLineEdit();
    diagSearchEdit->setPlaceholderText("Поиск диагнозов...");
    diagSearchEdit->addAction(QIcon(":/images/icon-diagnosis.svg"), QLineEdit::LeadingPosition);
    diagLayout->addWidget(diagSearchEdit);
    diagTable = new QTableWidget();
    diagTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    diagTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    diagTable->setSelectionMode(QAbstractItemView::SingleSelection);
    diagTable->setColumnCount(2);
    diagTable->setHorizontalHeaderLabels({"ID", "Название"});
    diagLayout->addWidget(diagTable);
    QHBoxLayout *diagActions = new QHBoxLayout();
    addDiagBtn = new QPushButton("Добавить");
    addDiagBtn->setIcon(QIcon(":/images/icon-add.svg"));
    addDiagBtn->setIconSize(QSize(16,16));
    editDiagBtn = new QPushButton("Изменить");
    editDiagBtn->setIcon(QIcon(":/images/icon-edit.svg"));
    editDiagBtn->setIconSize(QSize(16,16));
    deleteDiagBtn = new QPushButton("Удалить");
    deleteDiagBtn->setIcon(QIcon(":/images/icon-trash.svg"));
    deleteDiagBtn->setIconSize(QSize(16,16));
    diagActions->addWidget(addDiagBtn);
    diagActions->addWidget(editDiagBtn);
    diagActions->addWidget(deleteDiagBtn);
    diagActions->addStretch();
    diagLayout->addLayout(diagActions);
    dirLayout->addWidget(diagBox);

    dirLayout->addStretch();
    tabs->addTab(directoriesTab, "Справочники");

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
    
    connect(addSpecBtn, &QPushButton::clicked, this, &AdminWidget::onAddSpecialization);
    connect(editSpecBtn, &QPushButton::clicked, this, &AdminWidget::onEditSpecialization);
    connect(deleteSpecBtn, &QPushButton::clicked, this, &AdminWidget::onDeleteSpecialization);
    connect(specsSearchEdit, &QLineEdit::textChanged, this, &AdminWidget::onSpecializationsFilterChanged);

    connect(addRoomBtn, &QPushButton::clicked, this, &AdminWidget::onAddRoom);
    connect(editRoomBtn, &QPushButton::clicked, this, &AdminWidget::onEditRoom);
    connect(deleteRoomBtn, &QPushButton::clicked, this, &AdminWidget::onDeleteRoom);
    connect(roomsSearchEdit, &QLineEdit::textChanged, this, &AdminWidget::onRoomsFilterChanged);

    connect(addDiagBtn, &QPushButton::clicked, this, &AdminWidget::onAddDiagnosis);
    connect(editDiagBtn, &QPushButton::clicked, this, &AdminWidget::onEditDiagnosis);
    connect(deleteDiagBtn, &QPushButton::clicked, this, &AdminWidget::onDeleteDiagnosis);
    connect(diagSearchEdit, &QLineEdit::textChanged, this, &AdminWidget::onDiagnosesFilterChanged);
    
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

void AdminWidget::loadSpecializations() {
    specsTable->setRowCount(0);
    allSpecializations = dataManager->getAllSpecializations();
    specsTable->setRowCount(allSpecializations.size());
    for (int i = 0; i < allSpecializations.size(); ++i) {
        const Specialization &s = allSpecializations[i];
        specsTable->setItem(i, 0, new QTableWidgetItem(QString::number(s.id_spec)));
        specsTable->setItem(i, 1, new QTableWidgetItem(s.name));
    }
    specsTable->resizeColumnsToContents();
    specsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    specsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    specsSearchEdit->blockSignals(true);
    specsSearchEdit->clear();
    specsSearchEdit->blockSignals(false);
}

void AdminWidget::loadRooms() {
    roomsTable->setRowCount(0);
    allRooms = dataManager->getAllRooms();
    roomsTable->setRowCount(allRooms.size());
    for (int i = 0; i < allRooms.size(); ++i) {
        const Room &r = allRooms[i];
        roomsTable->setItem(i, 0, new QTableWidgetItem(QString::number(r.id_room)));
        roomsTable->setItem(i, 1, new QTableWidgetItem(r.room_number));
    }
    roomsTable->resizeColumnsToContents();
    roomsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    roomsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    roomsSearchEdit->blockSignals(true);
    roomsSearchEdit->clear();
    roomsSearchEdit->blockSignals(false);
}

void AdminWidget::loadDiagnoses() {
    diagTable->setRowCount(0);
    allDiagnoses = dataManager->getAllDiagnoses();
    diagTable->setRowCount(allDiagnoses.size());
    for (int i = 0; i < allDiagnoses.size(); ++i) {
        const Diagnosis &d = allDiagnoses[i];
        diagTable->setItem(i, 0, new QTableWidgetItem(QString::number(d.id_diagnosis)));
        diagTable->setItem(i, 1, new QTableWidgetItem(d.name));
    }
    diagTable->resizeColumnsToContents();
    diagTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    diagTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    diagSearchEdit->blockSignals(true);
    diagSearchEdit->clear();
    diagSearchEdit->blockSignals(false);
}

static bool nameExistsCaseInsensitive(const QString &value, const QList<QString> &existing, int skipIndex = -1) {
    for (int i = 0; i < existing.size(); ++i) {
        if (i == skipIndex) continue;
        if (existing[i].compare(value, Qt::CaseInsensitive) == 0) return true;
    }
    return false;
}

void AdminWidget::onAddSpecialization() {
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить специализацию");
    QFormLayout form(&dlg);
    QLineEdit *name = new QLineEdit();
    form.addRow("Название:", name);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    QString n = name->text().trimmed();
    if (n.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Введите название специализации"); return; }
    QList<QString> names;
    for (const auto &s : dataManager->getAllSpecializations()) names.append(s.name);
    if (nameExistsCaseInsensitive(n, names)) { QMessageBox::warning(this, "Ошибка", "Такая специализация уже существует"); return; }
    Specialization s; s.id_spec = dataManager->getNextSpecializationId(); s.name = n;
    dataManager->addSpecialization(s);
    loadSpecializations();
}

void AdminWidget::onEditSpecialization() {
    QTableWidgetItem *it = specsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите специализацию"); return; }
    int id = specsTable->item(it->row(), 0)->text().toInt();
    Specialization spec = dataManager->getSpecializationById(id);
    QDialog dlg(this);
    dlg.setWindowTitle("Изменить специализацию");
    QFormLayout form(&dlg);
    QLineEdit *name = new QLineEdit(spec.name);
    form.addRow("Название:", name);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    QString n = name->text().trimmed();
    if (n.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Введите название специализации"); return; }
    QList<QString> names;
    for (const auto &s : dataManager->getAllSpecializations()) names.append(s.name);
    if (nameExistsCaseInsensitive(n, names, -1) && n.compare(spec.name, Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this, "Ошибка", "Такая специализация уже существует");
        return;
    }
    spec.name = n;
    dataManager->updateSpecialization(spec);
    loadSpecializations();
    loadDoctors(); // обновить отображение названий у врачей
}

void AdminWidget::onDeleteSpecialization() {
    QTableWidgetItem *it = specsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите специализацию"); return; }
    int id = specsTable->item(it->row(), 0)->text().toInt();
    if (dataManager->isSpecializationUsed(id)) {
        QMessageBox::warning(this, "Нельзя удалить", "Специализация назначена врачам. Снимите назначение перед удалением.");
        return;
    }
    if (QMessageBox::question(this, "Подтвердите", "Удалить специализацию?") != QMessageBox::Yes) return;
    dataManager->deleteSpecialization(id);
    loadSpecializations();
}

void AdminWidget::onAddRoom() {
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить кабинет");
    QFormLayout form(&dlg);
    QLineEdit *number = new QLineEdit();
    form.addRow("Номер кабинета:", number);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    QString num = number->text().trimmed();
    if (num.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Введите номер кабинета"); return; }
    QList<QString> numbers;
    for (const auto &r : dataManager->getAllRooms()) numbers.append(r.room_number);
    if (nameExistsCaseInsensitive(num, numbers)) { QMessageBox::warning(this, "Ошибка", "Кабинет с таким номером уже существует"); return; }
    Room r; r.id_room = dataManager->getNextRoomId(); r.room_number = num;
    dataManager->addRoom(r);
    loadRooms();
}

void AdminWidget::onEditRoom() {
    QTableWidgetItem *it = roomsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите кабинет"); return; }
    int id = roomsTable->item(it->row(), 0)->text().toInt();
    Room room = dataManager->getRoomById(id);
    QDialog dlg(this);
    dlg.setWindowTitle("Изменить кабинет");
    QFormLayout form(&dlg);
    QLineEdit *number = new QLineEdit(room.room_number);
    form.addRow("Номер кабинета:", number);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    QString num = number->text().trimmed();
    if (num.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Введите номер кабинета"); return; }
    QList<QString> numbers;
    for (const auto &r : dataManager->getAllRooms()) numbers.append(r.room_number);
    if (nameExistsCaseInsensitive(num, numbers, -1) && num.compare(room.room_number, Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this, "Ошибка", "Кабинет с таким номером уже существует");
        return;
    }
    room.room_number = num;
    dataManager->updateRoom(room);
    loadRooms();
}

void AdminWidget::onDeleteRoom() {
    QTableWidgetItem *it = roomsTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите кабинет"); return; }
    int id = roomsTable->item(it->row(), 0)->text().toInt();
    if (dataManager->isRoomUsed(id)) {
        QMessageBox::warning(this, "Нельзя удалить", "Кабинет используется в расписаниях. Освободите слоты перед удалением.");
        return;
    }
    if (QMessageBox::question(this, "Подтвердите", "Удалить кабинет?") != QMessageBox::Yes) return;
    dataManager->deleteRoom(id);
    loadRooms();
}

void AdminWidget::onAddDiagnosis() {
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить диагноз");
    QFormLayout form(&dlg);
    QLineEdit *name = new QLineEdit();
    form.addRow("Название:", name);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    QString n = name->text().trimmed();
    if (n.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Введите название диагноза"); return; }
    QList<QString> names;
    for (const auto &d : dataManager->getAllDiagnoses()) names.append(d.name);
    if (nameExistsCaseInsensitive(n, names)) { QMessageBox::warning(this, "Ошибка", "Такой диагноз уже существует"); return; }
    Diagnosis d; d.id_diagnosis = dataManager->getNextDiagnosisId(); d.name = n;
    dataManager->addDiagnosis(d);
    loadDiagnoses();
}

void AdminWidget::onEditDiagnosis() {
    QTableWidgetItem *it = diagTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите диагноз"); return; }
    int id = diagTable->item(it->row(), 0)->text().toInt();
    Diagnosis d = dataManager->getDiagnosisById(id);
    QDialog dlg(this);
    dlg.setWindowTitle("Изменить диагноз");
    QFormLayout form(&dlg);
    QLineEdit *name = new QLineEdit(d.name);
    form.addRow("Название:", name);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dlg);
    form.addRow(&buttons);
    connect(&buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted) return;
    QString n = name->text().trimmed();
    if (n.isEmpty()) { QMessageBox::warning(this, "Ошибка", "Введите название диагноза"); return; }
    QList<QString> names;
    for (const auto &dd : dataManager->getAllDiagnoses()) names.append(dd.name);
    if (nameExistsCaseInsensitive(n, names, -1) && n.compare(d.name, Qt::CaseInsensitive) != 0) {
        QMessageBox::warning(this, "Ошибка", "Такой диагноз уже существует");
        return;
    }
    d.name = n;
    dataManager->updateDiagnosis(d);
    loadDiagnoses();
}

void AdminWidget::onDeleteDiagnosis() {
    QTableWidgetItem *it = diagTable->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите диагноз"); return; }
    int id = diagTable->item(it->row(), 0)->text().toInt();
    if (dataManager->isDiagnosisUsed(id)) {
        QMessageBox::warning(this, "Нельзя удалить", "Диагноз уже используется в рецептах. Удалите или обновите связанные записи.");
        return;
    }
    if (QMessageBox::question(this, "Подтвердите", "Удалить диагноз?") != QMessageBox::Yes) return;
    dataManager->deleteDiagnosis(id);
    loadDiagnoses();
}

void AdminWidget::onSpecializationsFilterChanged(const QString &text) {
    for (int row = 0; row < specsTable->rowCount(); ++row) {
        bool match = text.isEmpty();
        if (!match) {
            for (int col = 0; col < specsTable->columnCount(); ++col) {
                QTableWidgetItem *item = specsTable->item(row, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) { match = true; break; }
            }
        }
        specsTable->setRowHidden(row, !match);
    }
}

void AdminWidget::onRoomsFilterChanged(const QString &text) {
    for (int row = 0; row < roomsTable->rowCount(); ++row) {
        bool match = text.isEmpty();
        if (!match) {
            for (int col = 0; col < roomsTable->columnCount(); ++col) {
                QTableWidgetItem *item = roomsTable->item(row, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) { match = true; break; }
            }
        }
        roomsTable->setRowHidden(row, !match);
    }
}

void AdminWidget::onDiagnosesFilterChanged(const QString &text) {
    for (int row = 0; row < diagTable->rowCount(); ++row) {
        bool match = text.isEmpty();
        if (!match) {
            for (int col = 0; col < diagTable->columnCount(); ++col) {
                QTableWidgetItem *item = diagTable->item(row, col);
                if (item && item->text().contains(text, Qt::CaseInsensitive)) { match = true; break; }
            }
        }
        diagTable->setRowHidden(row, !match);
    }
}

#include "admins/adminwidget.moc"
