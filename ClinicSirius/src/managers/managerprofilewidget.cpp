#include "managers/managerprofilewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include "patients/createpatientdialog.h"
#include "managers/managerscheduleviewer.h"

ManagerProfileWidget::ManagerProfileWidget(QWidget *parent)
    : QWidget(parent), m_dataManager(QString()) {
    buildUI();
}

void ManagerProfileWidget::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);
    QHBoxLayout *header = new QHBoxLayout();

    nameLabel = new QLabel("Менеджер: —");
    nameLabel->setStyleSheet("font-weight:bold; font-size:16px;");
    header->addWidget(nameLabel);
    header->addStretch();
    main->addLayout(header);

    emailLabel = new QLabel("Email: —");
    main->addWidget(emailLabel);

    registerPatientBtn = new QPushButton("➕ Зарегистрировать пациента");
    attachFamilyBtn = new QPushButton("🔗 Прикрепить к семье");
    viewScheduleBtn = new QPushButton("📋 Посмотреть расписание врачей");

    QHBoxLayout *actions = new QHBoxLayout();
    actions->addWidget(registerPatientBtn);
    actions->addWidget(attachFamilyBtn);
    actions->addWidget(viewScheduleBtn);
    actions->addStretch();
    main->addLayout(actions);

    connect(registerPatientBtn, &QPushButton::clicked, this, &ManagerProfileWidget::onRegisterPatient);
    connect(attachFamilyBtn, &QPushButton::clicked, this, &ManagerProfileWidget::onAttachToFamily);
    connect(viewScheduleBtn, &QPushButton::clicked, this, &ManagerProfileWidget::onViewClinicSchedule);
}

void ManagerProfileWidget::setUser(const LoginUser &user) {
    m_user = user;
    loadManagerInfo();
}

void ManagerProfileWidget::loadManagerInfo() {
    Manager m = m_dataManager.getManagerById(m_user.id);
    nameLabel->setText(QString("Менеджер: %1").arg(m.fullName()));
    emailLabel->setText(QString("Email: %1").arg(m.email));
}

void ManagerProfileWidget::onRegisterPatient() {
    // Reuse existing CreatePatientDialog
    CreatePatientDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Patient p = dlg.getCreatedPatient();
        p.id_patient = m_dataManager.getNextPatientId();
        // Password should be hashed; CreatePatientDialog already handles
        m_dataManager.addPatient(p);
        QMessageBox::information(this, "Готово", "Пациент успешно зарегистрирован");
    }
}

void ManagerProfileWidget::onAttachToFamily() {
    bool ok = false;
    int parentId = QInputDialog::getInt(this, "ID родителя", "Введите ID родителя:", 0, 0, 1000000, 1, &ok);
    if (!ok) return;
    int childId = QInputDialog::getInt(this, "ID ребёнка", "Введите ID ребёнка:", 0, 0, 1000000, 1, &ok);
    if (!ok) return;

    // Create PatientGroup entry
    PatientGroup pg;
    pg.id_patient_group = m_dataManager.getNextPatientGroupId();
    pg.id_parent = parentId;
    pg.id_child = childId;
    pg.family_head = parentId;
    m_dataManager.addFamilyMember(pg);
    QMessageBox::information(this, "Готово", "Пациент прикреплён к семье");
}

void ManagerProfileWidget::onViewClinicSchedule() {
    ManagerScheduleViewer dlg(this);
    dlg.exec();
}
