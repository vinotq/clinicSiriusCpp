#include "patients/familyviewerwidget.h"
#include "patients/createpatientdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QInputDialog>
#include <QCompleter>
#include <QDialog>
#include <QScrollArea>
#include <QFormLayout>
#include <algorithm>

FamilyViewerWidget::FamilyViewerWidget(QWidget *parent)
    : QWidget(parent), m_dataManager(QString()), m_selectedPatientId(-1) {
    buildUI();
}

void FamilyViewerWidget::buildUI() {
    setWindowTitle("Управление семьями пациентов");
    resize(700, 600);

    QVBoxLayout *main = new QVBoxLayout(this);

    // ===== Patient Selector Section =====
    QGroupBox *patientGroup = new QGroupBox("Выбор пациента для управления семьей");
    QVBoxLayout *patientLayout = new QVBoxLayout(patientGroup);

    // Search field
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("Поиск пациента:");
    m_patientSearchEdit = new QLineEdit();
    m_patientSearchEdit->setPlaceholderText("Введите имя или фамилию...");
    m_patientSearchEdit->setMaximumWidth(300);
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_patientSearchEdit);
    searchLayout->addStretch();
    patientLayout->addLayout(searchLayout);

    // Patient combo box
    QHBoxLayout *comboLayout = new QHBoxLayout();
    QLabel *patientLabel = new QLabel("Выбранный пациент:");
    m_patientComboBox = new QComboBox();
    m_patientComboBox->setMinimumWidth(300);
    comboLayout->addWidget(patientLabel);
    comboLayout->addWidget(m_patientComboBox);
    comboLayout->addStretch();
    patientLayout->addLayout(comboLayout);

    // Patient info
    m_patientInfoLabel = new QLabel("Выберите пациента для управления его семьей");
    m_patientInfoLabel->setStyleSheet("color: #666; font-style: italic;");
    patientLayout->addWidget(m_patientInfoLabel);

    main->addWidget(patientGroup);

    // ===== Family Info and Control Section =====
    QGroupBox *familyGroup = new QGroupBox("Управление семьей пациента");
    QVBoxLayout *familyLayout = new QVBoxLayout(familyGroup);

    // Family head info
    m_familyHeadLabel = new QLabel("Глава семьи: ---");
    m_familyHeadLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    familyLayout->addWidget(m_familyHeadLabel);

    // Family members list
    QLabel *membersLabel = new QLabel("Члены семьи:");
    familyLayout->addWidget(membersLabel);

    m_familyList = new QListWidget();
    m_familyList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_familyList->setMinimumHeight(250);
    familyLayout->addWidget(m_familyList);

    // Action buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_addBtn = new QPushButton("+ Добавить члена");
    m_editBtn = new QPushButton("✎ Редактировать");
    m_removeBtn = new QPushButton("✕ Удалить");

    m_addBtn->setEnabled(false);
    m_editBtn->setEnabled(false);
    m_removeBtn->setEnabled(false);

    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_removeBtn);
    btnLayout->addStretch();

    familyLayout->addLayout(btnLayout);

    // Status label
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: #2d7f2d;");
    familyLayout->addWidget(m_statusLabel);

    main->addWidget(familyGroup);

    // Connections
    connect(m_patientSearchEdit, &QLineEdit::textChanged, this, &FamilyViewerWidget::onSearchPatient);
    connect(m_patientComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &FamilyViewerWidget::onPatientSelected);
    connect(m_addBtn, &QPushButton::clicked, this, &FamilyViewerWidget::onAddMember);
    connect(m_editBtn, &QPushButton::clicked, this, &FamilyViewerWidget::onEditMember);
    connect(m_removeBtn, &QPushButton::clicked, this, &FamilyViewerWidget::onRemoveMember);
    connect(m_familyList, &QListWidget::itemDoubleClicked, this, &FamilyViewerWidget::onMemberDoubleClicked);
    connect(m_familyList, &QListWidget::itemSelectionChanged, this, [this]() {
        bool hasSelection = m_familyList->currentItem() != nullptr;
        m_editBtn->setEnabled(hasSelection && m_selectedPatientId > 0);
        m_removeBtn->setEnabled(hasSelection && m_selectedPatientId > 0);
    });
}

void FamilyViewerWidget::setUser(const LoginUser &user) {
    m_currentUser = user;
    // Remove patient-only restriction - now works for both managers and patients
    populatePatientSelector();
}

void FamilyViewerWidget::setSelectedPatient(int patientId) {
    m_selectedPatientId = patientId;
    if (patientId > 0) {
        updatePatientSelector(patientId);
        loadFamilyMembers();
    }
}

void FamilyViewerWidget::populatePatientSelector() {
    m_patientComboBox->clear();
    
    // Load all patients
    QList<Patient> allPatients = m_dataManager.getAllPatients();
    std::sort(allPatients.begin(), allPatients.end(), 
              [](const Patient &a, const Patient &b) { return a.fullName().toLower() < b.fullName().toLower(); });

    for (const Patient &p : allPatients) {
        QString displayName = QString("%1 (ID: %2)").arg(p.fullName(), QString::number(p.id_patient));
        m_patientComboBox->addItem(displayName, p.id_patient);
    }
}

void FamilyViewerWidget::updatePatientSelector(int selectedPatientId) {
    // Find and select patient in combo box
    for (int i = 0; i < m_patientComboBox->count(); ++i) {
        if (m_patientComboBox->itemData(i).toInt() == selectedPatientId) {
            m_patientComboBox->setCurrentIndex(i);
            return;
        }
    }
}

void FamilyViewerWidget::onSearchPatient(const QString &text) {
    // Filter combo box based on search text
    if (text.isEmpty()) {
        populatePatientSelector();
        return;
    }

    m_patientComboBox->clear();
    QList<Patient> allPatients = m_dataManager.getAllPatients();
    
    for (const Patient &p : allPatients) {
        if (p.fullName().toLower().contains(text.toLower())) {
            QString displayName = QString("%1 (ID: %2)").arg(p.fullName(), QString::number(p.id_patient));
            m_patientComboBox->addItem(displayName, p.id_patient);
        }
    }
}

void FamilyViewerWidget::onPatientSelected(int index) {
    if (index < 0) return;

    m_selectedPatientId = m_patientComboBox->itemData(index).toInt();
    if (m_selectedPatientId > 0) {
        loadFamilyMembers();
    }
}

void FamilyViewerWidget::loadFamilyMembers() {
    if (m_selectedPatientId <= 0) {
        m_familyHeadLabel->setText("Глава семьи: ---");
        m_patientInfoLabel->setText("Выберите пациента для управления его семьей");
        m_statusLabel->clear();
        m_addBtn->setEnabled(false);
        refreshFamilyList();
        return;
    }

    // REQ-006: Display family headed by selected patient
    Patient headPatient = m_dataManager.getPatientById(m_selectedPatientId);
    if (headPatient.id_patient <= 0) {
        m_familyHeadLabel->setText("Глава семьи: Пациент не найден");
        m_statusLabel->setText("Ошибка: пациент не найден в системе");
        m_addBtn->setEnabled(false);
        return;
    }

    m_familyHeadLabel->setText(QString("Глава семьи: %1 (ID: %2)").arg(headPatient.fullName(), QString::number(m_selectedPatientId)));
    m_patientInfoLabel->setText(QString("Управление семьей пациента %1").arg(headPatient.fullName()));
    m_addBtn->setEnabled(true);
    
    refreshFamilyList();
}

void FamilyViewerWidget::refreshFamilyList() {
    m_familyList->clear();
    m_statusLabel->clear();
    m_editBtn->setEnabled(false);
    m_removeBtn->setEnabled(false);

    if (m_selectedPatientId <= 0) {
        m_familyList->addItem("Пациент не выбран");
        return;
    }

    // Get all family members where selected patient is the head
    QList<PatientGroup> familyMembers = m_dataManager.getPatientFamilyMembers(m_selectedPatientId);

    if (familyMembers.isEmpty()) {
        m_familyList->addItem("Нет членов семьи (кроме главы)");
        m_statusLabel->setText("Добавьте членов семьи с помощью кнопки 'Добавить члена'");
        return;
    }

    for (const PatientGroup &pg : familyMembers) {
        Patient child = m_dataManager.getPatientById(pg.id_child);
        QString displayName = child.fullName();
        QListWidgetItem *item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, pg.id_child);
        item->setData(Qt::UserRole + 1, pg.id_patient_group);
        m_familyList->addItem(item);
    }
}

void FamilyViewerWidget::onAddMember() {
    if (m_selectedPatientId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Сначала выберите пациента");
        return;
    }

    // Добавление только по поиску (без кода) с подсказками по ФИО
    QDialog dlg(this);
    dlg.setWindowTitle("Добавить члена семьи");
    dlg.resize(420, 150);

    QVBoxLayout *main = new QVBoxLayout(&dlg);
    QLabel *label = new QLabel("Найдите пациента по ФИО:");
    main->addWidget(label);

    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Начните вводить ФИО пациента...");

    // Собираем список всех пациентов для подсказок, кроме головы семьи
    QStringList patientNames;
    QList<Patient> allPatients = m_dataManager.getAllPatients();
    for (const Patient &p : allPatients) {
        if (p.id_patient != m_selectedPatientId) {
            patientNames << p.fullName();
        }
    }
    patientNames.sort();

    QCompleter *completer = new QCompleter(patientNames, &dlg);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    searchEdit->setCompleter(completer);

    main->addWidget(searchEdit);

    QHBoxLayout *btns = new QHBoxLayout();
    QPushButton *okBtn = new QPushButton("ОК");
    QPushButton *cancelBtn = new QPushButton("Отмена");
    btns->addStretch();
    btns->addWidget(okBtn);
    btns->addWidget(cancelBtn);
    main->addLayout(btns);

    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    const QString patientName = searchEdit->text().trimmed();
    if (patientName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите ФИО пациента");
        return;
    }

    // Ищем пациента по полному совпадению ФИО (без учёта регистра)
    int selectedId = -1;
    for (const Patient &p : allPatients) {
        if (p.fullName().toLower() == patientName.toLower() &&
            p.id_patient != m_selectedPatientId) {
            selectedId = p.id_patient;
            break;
        }
    }

    if (selectedId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Пациент не найден или это сама голова семьи");
        return;
    }

    if (m_dataManager.isFamilyMember(m_selectedPatientId, selectedId)) {
        QMessageBox::information(this, "Ошибка", "Этот пациент уже в семье");
        return;
    }

    PatientGroup pg;
    pg.id_patient_group = m_dataManager.getNextPatientGroupId();
    pg.id_parent = m_selectedPatientId;
    pg.id_child = selectedId;
    pg.family_head = m_selectedPatientId;

    m_dataManager.addFamilyMember(pg);
    m_statusLabel->setText("Пациент добавлен в семью");
    refreshFamilyList();
}

void FamilyViewerWidget::onEditMember() {
    // REQ-006: Edit family member
    QListWidgetItem *item = m_familyList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите члена семьи");
        return;
    }

    int patientId = item->data(Qt::UserRole).toInt();
    Patient p = m_dataManager.getPatientById(patientId);

    CreatePatientDialog dlg(this, &p);
    if (dlg.exec() == QDialog::Accepted) {
        m_statusLabel->setText("Данные члена семьи обновлены");
        refreshFamilyList();
    }
}

void FamilyViewerWidget::onRemoveMember() {
    // REQ-006: Remove family member with confirmation
    QListWidgetItem *item = m_familyList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите члена семьи для удаления");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Подтверждение", "Вы уверены, что хотите удалить этого члена из семьи?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int groupId = item->data(Qt::UserRole + 1).toInt();
        m_dataManager.removeFamilyMember(groupId);
        m_statusLabel->setText("Член семьи удален");
        refreshFamilyList();
    }
}

void FamilyViewerWidget::onMemberDoubleClicked(QListWidgetItem *item) {
    // REQ-006: Double-click to edit
    Q_UNUSED(item);
    onEditMember();
}
