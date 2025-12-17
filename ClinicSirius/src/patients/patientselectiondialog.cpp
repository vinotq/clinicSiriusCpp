#include "patientselectiondialog.h"
#include "patients/createpatientdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <algorithm>

PatientSelectionDialog::PatientSelectionDialog(QWidget *parent, const QList<Patient> &availablePatients)
    : QDialog(parent), m_dataManager(QString()) {
    setWindowTitle("Выбор пациента");
    setMinimumWidth(500);
    setMinimumHeight(400);

    // Use provided patients or load all
    if (!availablePatients.isEmpty()) {
        m_allPatients = availablePatients;
    } else {
        m_allPatients = m_dataManager.getAllPatients();
    }
    // Sort alphabetically
    std::sort(m_allPatients.begin(), m_allPatients.end(), [](const Patient &a, const Patient &b){
        return a.fullName().toLower() < b.fullName().toLower();
    });

    buildUI();
    updatePatientList();
}

void PatientSelectionDialog::buildUI() {
    QVBoxLayout *main = new QVBoxLayout(this);

    QLabel *title = new QLabel("Выберите пациента для записи:");
    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPointSize(11);
    title->setFont(titleFont);
    main->addWidget(title);

    // Search field
    QHBoxLayout *searchLay = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("Поиск:");
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Введите ФИО пациента...");
    searchLay->addWidget(searchLabel);
    searchLay->addWidget(m_searchEdit);
    main->addLayout(searchLay);

    // Patient list
    m_patientList = new QListWidget();
    m_patientList->setSelectionMode(QAbstractItemView::SingleSelection);
    main->addWidget(m_patientList);

    // Status label
    m_statusLabel = new QLabel();
    m_statusLabel->setStyleSheet("color: #666;");
    main->addWidget(m_statusLabel);

    // Buttons
    QHBoxLayout *btnLay = new QHBoxLayout();

    m_createBtn = new QPushButton("+ Создать нового");
    m_selfBtn = new QPushButton("👤 Выбрать себя");
    m_selectBtn = new QPushButton("✅ Выбрать");
    m_selectBtn->setEnabled(false);
    m_cancelBtn = new QPushButton("❌ Отмена");

    btnLay->addWidget(m_createBtn);
    btnLay->addWidget(m_selfBtn);
    btnLay->addStretch();
    btnLay->addWidget(m_selectBtn);
    btnLay->addWidget(m_cancelBtn);
    main->addLayout(btnLay);

    // Connections
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PatientSelectionDialog::onSearchTextChanged);
    connect(m_patientList, &QListWidget::itemClicked, this, &PatientSelectionDialog::onListItemSelected);
    connect(m_patientList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *){
        if (m_selectBtn->isEnabled()) accept();
    });
    connect(m_createBtn, &QPushButton::clicked, this, &PatientSelectionDialog::onCreateNew);
    connect(m_selfBtn, &QPushButton::clicked, this, [this](){
        // Try to find "Self" option or auto-select if current user is in available list
        if (!m_allPatients.isEmpty()) {
            m_patientList->setCurrentRow(0);
            onListItemSelected(m_patientList->item(0));
            m_selectBtn->setEnabled(true);
        }
    });
    connect(m_selectBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void PatientSelectionDialog::updatePatientList(const QString &filter) {
    m_patientList->clear();
    int count = 0;

    for (const Patient &p : m_allPatients) {
        QString fullName = p.fullName();
        if (filter.isEmpty() || fullName.toLower().contains(filter.toLower())) {
            QListWidgetItem *item = new QListWidgetItem(fullName);
            item->setData(Qt::UserRole, p.id_patient);
            m_patientList->addItem(item);
            count++;
        }
    }

    // Update status
    if (filter.isEmpty()) {
        m_statusLabel->setText(QString("Всего пациентов: %1").arg(m_allPatients.size()));
    } else {
        m_statusLabel->setText(QString("Найдено: %1").arg(count));
    }
}

void PatientSelectionDialog::onSearchTextChanged(const QString &text) {
    updatePatientList(text);
    m_selectBtn->setEnabled(false);
    m_selectedPatient = Patient();
}

void PatientSelectionDialog::onListItemSelected(QListWidgetItem *item) {
    if (!item) return;
    int id = item->data(Qt::UserRole).toInt();
    m_selectedPatient = m_dataManager.getPatientById(id);
    m_selectBtn->setEnabled(m_selectedPatient.id_patient > 0);
}

void PatientSelectionDialog::onCreateNew() {
    // Open patient creation dialog
    CreatePatientDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Patient p = dlg.getCreatedPatient();
        // Add to datamanager
        m_dataManager.addPatient(p);
        // Add to our list
        m_allPatients.append(p);
        std::sort(m_allPatients.begin(), m_allPatients.end(), [](const Patient &a, const Patient &b){
            return a.fullName().toLower() < b.fullName().toLower();
        });
        // Update display and select the new patient
        m_searchEdit->clear();
        updatePatientList();
        // Find and select the new patient
        for (int i = 0; i < m_patientList->count(); ++i) {
            QListWidgetItem *item = m_patientList->item(i);
            if (item->data(Qt::UserRole).toInt() == p.id_patient) {
                m_patientList->setCurrentItem(item);
                onListItemSelected(item);
                break;
            }
        }
        QMessageBox::information(this, "Готово", "Пациент создан и выбран для записи");
    }
}

Patient PatientSelectionDialog::getSelectedPatient() const {
    return m_selectedPatient;
}
