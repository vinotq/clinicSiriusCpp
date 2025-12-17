#include "managers/patientmanagementdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QDialog>
#include <QHeaderView>
#include <algorithm>

PatientManagementDialog::PatientManagementDialog(QWidget* parent)
    : QWidget(parent), m_dataManager(QString()) {
    setWindowTitle("Управление пациентами");
    resize(900, 600);

    buildUI();
}

void PatientManagementDialog::buildUI() {
    QVBoxLayout* main = new QVBoxLayout(this);
    
    // Toolbar with search and action buttons
    QHBoxLayout* top = new QHBoxLayout();
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Поиск по ФИО, email или ID...");
    
    m_createBtn = new QPushButton("+ Создать");
    m_editBtn = new QPushButton("✎ Редактировать");
    m_deleteBtn = new QPushButton("❌ Удалить");
    m_addToFamilyBtn = new QPushButton("👪 Добавить в семью");

    top->addWidget(m_searchEdit, 1);
    top->addWidget(m_createBtn);
    top->addWidget(m_editBtn);
    top->addWidget(m_deleteBtn);
    top->addWidget(m_addToFamilyBtn);
    main->addLayout(top);

    // Patient table
    m_patientTable = new QTableWidget();
    m_patientTable->setColumnCount(4);
    m_patientTable->setHorizontalHeaderLabels({"ID", "ФИО", "Email", "Телефон"});
    m_patientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_patientTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_patientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_patientTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_patientTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_patientTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_patientTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_patientTable->verticalHeader()->setVisible(false);
    main->addWidget(m_patientTable, 1);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &PatientManagementDialog::onSearchTextChanged);
    connect(m_createBtn, &QPushButton::clicked, this, &PatientManagementDialog::onCreatePatient);
    connect(m_editBtn, &QPushButton::clicked, this, &PatientManagementDialog::onEditPatient);
    connect(m_deleteBtn, &QPushButton::clicked, this, &PatientManagementDialog::onDeletePatient);
    connect(m_addToFamilyBtn, &QPushButton::clicked, this, &PatientManagementDialog::onAddToFamily);

    // Setup autocomplete for patient names
    QStringList patientNames;
    QList<Patient> allPatients = m_dataManager.getAllPatients();
    for (const Patient &p : allPatients) {
        patientNames << p.fullName();
    }
    patientNames.sort();
    m_patientNameCompleter = new QCompleter(patientNames, this);
    m_patientNameCompleter->setCaseSensitivity(Qt::CaseInsensitive);

    refreshList();
}

void PatientManagementDialog::refreshList(const QString &filter) {
    m_patientTable->setRowCount(0);
    
    QList<Patient> patients = m_dataManager.getAllPatients();
    // Sort patients alphabetically by full name
    std::sort(patients.begin(), patients.end(), [](const Patient &a, const Patient &b){
        return a.fullName().toLower() < b.fullName().toLower();
    });

    for (const Patient &p : patients) {
        QString name = p.fullName();
        QString email = p.email;
        QString idStr = QString::number(p.id_patient);
        
        // Apply filter
        if (!filter.isEmpty() && 
            !name.toLower().contains(filter.toLower()) &&
            !email.toLower().contains(filter.toLower()) &&
            !idStr.contains(filter)) {
            continue;
        }

        int row = m_patientTable->rowCount();
        m_patientTable->insertRow(row);

        // ID column
        QTableWidgetItem* idItem = new QTableWidgetItem(idStr);
        idItem->setData(Qt::UserRole, p.id_patient);
        m_patientTable->setItem(row, 0, idItem);

        // Name column
        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        m_patientTable->setItem(row, 1, nameItem);

        // Email column
        QTableWidgetItem* emailItem = new QTableWidgetItem(email);
        m_patientTable->setItem(row, 2, emailItem);

        // Phone column
        QTableWidgetItem* phoneItem = new QTableWidgetItem(p.phone_number);
        m_patientTable->setItem(row, 3, phoneItem);
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
        QMessageBox::information(this, "Успешно", "Пациент создан");
    }
}

void PatientManagementDialog::onEditPatient() {
    int row = m_patientTable->currentRow();
    if (row < 0) { 
        QMessageBox::warning(this, "Ошибка", "Выберите пациента для редактирования"); 
        return; 
    }
    
    int id = m_patientTable->item(row, 0)->data(Qt::UserRole).toInt();
    Patient p = m_dataManager.getPatientById(id);
    
    CreatePatientDialog dlg(this, &p);
    if (dlg.exec() == QDialog::Accepted) {
        Patient updated = dlg.getCreatedPatient();
        updated.id_patient = id;
        m_dataManager.updatePatient(updated);
        refreshList(m_searchEdit->text());
        QMessageBox::information(this, "Успешно", "Пациент обновлен");
    }
}

void PatientManagementDialog::onDeletePatient() {
    int row = m_patientTable->currentRow();
    if (row < 0) { 
        QMessageBox::warning(this, "Ошибка", "Выберите пациента для удаления"); 
        return; 
    }
    
    int id = m_patientTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString name = m_patientTable->item(row, 1)->text();
    
    if (QMessageBox::question(this, "Подтвердите", 
        QString("Удалить пациента '%1'?").arg(name)) != QMessageBox::Yes) {
        return;
    }
    
    m_dataManager.deletePatient(id);
    refreshList(m_searchEdit->text());
    QMessageBox::information(this, "Успешно", "Пациент удален");
}

void PatientManagementDialog::onAddToFamily() {
    int row = m_patientTable->currentRow();
    if (row < 0) { 
        QMessageBox::warning(this, "Ошибка", "Выберите пациента (ребенка)"); 
        return; 
    }
    
    int childId = m_patientTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString childName = m_patientTable->item(row, 1)->text();
    
    showAddToFamilyDialog();
}

void PatientManagementDialog::showAddToFamilyDialog() {
    // Get currently selected patient (child)
    int row = m_patientTable->currentRow();
    if (row < 0) return;
    
    int childId = m_patientTable->item(row, 0)->data(Qt::UserRole).toInt();
    QString childName = m_patientTable->item(row, 1)->text();

    // Create a dialog to select parent
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить в семью");
    dlg.resize(400, 150);
    
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    
    QLabel* label = new QLabel(QString("Найти родителя для пациента '%1':").arg(childName));
    layout->addWidget(label);
    
    QLineEdit* parentSearchEdit = new QLineEdit();
    parentSearchEdit->setPlaceholderText("Начните вводить имя родителя...");
    
    // Setup autocomplete for parent search
    QStringList parentNames;
    QList<Patient> allPatients = m_dataManager.getAllPatients();
    for (const Patient &p : allPatients) {
        if (p.id_patient != childId) {  // Exclude the child itself
            parentNames << p.fullName();
        }
    }
    parentNames.sort();
    QCompleter* parentCompleter = new QCompleter(parentNames, &dlg);
    parentCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    parentSearchEdit->setCompleter(parentCompleter);
    
    layout->addWidget(parentSearchEdit);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton("ОК");
    QPushButton* cancelBtn = new QPushButton("Отмена");
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);
    
    connect(okBtn, &QPushButton::clicked, [&dlg]() { dlg.accept(); });
    connect(cancelBtn, &QPushButton::clicked, [&dlg]() { dlg.reject(); });
    
    if (dlg.exec() != QDialog::Accepted) return;
    
    QString parentName = parentSearchEdit->text().trimmed();
    if (parentName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя родителя");
        return;
    }

    // Find parent by name
    QList<Patient> patients = m_dataManager.getAllPatients();
    int parentId = -1;
    for (const Patient &p : patients) {
        if (p.fullName().toLower() == parentName.toLower()) { 
            parentId = p.id_patient; 
            break; 
        }
    }
    
    if (parentId <= 0) {
        QMessageBox::warning(this, "Не найдено", 
            QString("Пациент с именем '%1' не найден").arg(parentName));
        return;
    }

    if (parentId == childId) {
        QMessageBox::warning(this, "Ошибка", "Нельзя добавить пациента в семью с самим собой");
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
    QMessageBox::information(this, "Успешно", 
        QString("Пациент '%1' добавлен в семью к пациенту '%2'")
            .arg(childName, parentName));
}
