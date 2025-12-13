#include "managers/patientmanagementdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>

PatientManagementDialog::PatientManagementDialog(QWidget* parent)
    : QDialog(parent), m_dataManager(QString()) {
    setWindowTitle("Управление пациентами");
    resize(700, 500);

    QVBoxLayout* main = new QVBoxLayout(this);
    QHBoxLayout* top = new QHBoxLayout();
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Поиск по ФИО...");
    m_createBtn = new QPushButton("+ Создать");
    m_editBtn = new QPushButton("✎ Редактировать");
    m_deleteBtn = new QPushButton("🗑 Удалить");
    m_addToFamilyBtn = new QPushButton("👪 Добавить в семью");

    top->addWidget(m_searchEdit, 1);
    top->addWidget(m_createBtn);
    top->addWidget(m_editBtn);
    top->addWidget(m_deleteBtn);
    top->addWidget(m_addToFamilyBtn);
    main->addLayout(top);

    m_list = new QListWidget();
    main->addWidget(m_list, 1);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &PatientManagementDialog::onSearchTextChanged);
    connect(m_createBtn, &QPushButton::clicked, this, &PatientManagementDialog::onCreatePatient);
    connect(m_editBtn, &QPushButton::clicked, this, &PatientManagementDialog::onEditPatient);
    connect(m_deleteBtn, &QPushButton::clicked, this, &PatientManagementDialog::onDeletePatient);
    connect(m_addToFamilyBtn, &QPushButton::clicked, this, &PatientManagementDialog::onAddToFamily);

    refreshList();
}

void PatientManagementDialog::refreshList(const QString &filter) {
    m_list->clear();
    QList<Patient> patients = m_dataManager.getAllPatients();
    for (const Patient &p : patients) {
        QString name = p.fullName();
        if (filter.isEmpty() || name.toLower().contains(filter.toLower())) {
            QListWidgetItem* it = new QListWidgetItem(name);
            it->setData(Qt::UserRole, p.id_patient);
            m_list->addItem(it);
        }
    }
}

void PatientManagementDialog::onSearchTextChanged(const QString &text) {
    refreshList(text);
}

void PatientManagementDialog::onCreatePatient() {
    CreatePatientDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Patient p = dlg.getCreatedPatient();
        m_dataManager.addPatient(p);
        refreshList(m_searchEdit->text());
    }
}

void PatientManagementDialog::onEditPatient() {
    QListWidgetItem* it = m_list->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите пациента для редактирования"); return; }
    int id = it->data(Qt::UserRole).toInt();
    Patient p = m_dataManager.getPatientById(id);
    CreatePatientDialog dlg(this, &p);
    if (dlg.exec() == QDialog::Accepted) {
        Patient updated = dlg.getCreatedPatient();
        updated.id_patient = id;
        m_dataManager.updatePatient(updated);
        refreshList(m_searchEdit->text());
    }
}

void PatientManagementDialog::onDeletePatient() {
    QListWidgetItem* it = m_list->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите пациента для удаления"); return; }
    int id = it->data(Qt::UserRole).toInt();
    if (QMessageBox::question(this, "Подтвердите", "Удалить пациента?") != QMessageBox::Yes) return;
    m_dataManager.deletePatient(id);
    refreshList(m_searchEdit->text());
}

void PatientManagementDialog::onAddToFamily() {
    QListWidgetItem* it = m_list->currentItem();
    if (!it) { QMessageBox::warning(this, "Ошибка", "Выберите пациента"); return; }
    int childId = it->data(Qt::UserRole).toInt();
    bool ok;
    QString parentFIO = QInputDialog::getText(this, "Добавить в семью", "ФИО родителя (поиск):", QLineEdit::Normal, "", &ok);
    if (!ok || parentFIO.trimmed().isEmpty()) return;

    // Find parent by FIO
    QList<Patient> patients = m_dataManager.getAllPatients();
    int parentId = -1;
    for (const Patient &p : patients) {
        if (p.fullName().toLower() == parentFIO.toLower()) { parentId = p.id_patient; break; }
    }
    if (parentId <= 0) {
        QMessageBox::warning(this, "Не найдено", "Пациент с таким ФИО не найден");
        return;
    }

    if (m_dataManager.isFamilyMember(parentId, childId)) {
        QMessageBox::information(this, "Инфо", "Эти пациенты уже состоят в семье");
        return;
    }

    PatientGroup pg;
    pg.id_patient_group = m_dataManager.getNextPatientGroupId();
    pg.id_parent = parentId;
    pg.id_child = childId;
    m_dataManager.addFamilyMember(pg);
    QMessageBox::information(this, "Готово", "Пациент добавлен в семью");
}
