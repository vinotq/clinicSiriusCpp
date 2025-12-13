#include "patienthistorywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include "../common/datamanager.h"

PatientHistoryWidget::PatientHistoryWidget(QWidget *parent)
    : QWidget(parent), m_dataManager(QString()) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20,20,20,20);

    QLabel *title = new QLabel("История приёмов пациентов");
    QFont f; f.setPointSize(14); f.setBold(true);
    title->setFont(f);
    mainLayout->addWidget(title);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("Введите ФИО пациента или часть имени");
    m_searchButton = new QPushButton("Найти");
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    mainLayout->addLayout(searchLayout);

    m_patientsList = new QListWidget();
    m_patientsList->setMinimumHeight(150);
    m_patientsList->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(new QLabel("Найденные пациенты:"));
    mainLayout->addWidget(m_patientsList);

    m_appointmentsList = new QListWidget();
    m_appointmentsList->setMinimumHeight(200);
    mainLayout->addWidget(new QLabel("Приёмы пациента:"));
    mainLayout->addWidget(m_appointmentsList);

    connect(m_searchButton, &QPushButton::clicked, this, &PatientHistoryWidget::onSearchClicked);
    connect(m_patientsList, &QListWidget::itemClicked, this, &PatientHistoryWidget::onPatientSelected);
}

void PatientHistoryWidget::onSearchClicked() {
    m_patientsList->clear();
    m_appointmentsList->clear();

    QString query = m_searchEdit->text().trimmed();
    if (query.isEmpty()) {
        QMessageBox::information(this, "Пустой запрос", "Введите фамилию или имя для поиска.");
        return;
    }

    QList<Patient> all = m_dataManager.getAllPatients();
    for (const auto &p : all) {
        QString fullname = p.fullName();
        if (fullname.contains(query, Qt::CaseInsensitive)) {
            QListWidgetItem *it = new QListWidgetItem(fullname);
            it->setData(Qt::UserRole, p.id_patient);
            m_patientsList->addItem(it);
        }
    }

    if (m_patientsList->count() == 0) {
        QMessageBox::information(this, "Не найдено", "Пациенты не найдены по заданному запросу.");
    }
}

void PatientHistoryWidget::onPatientSelected(QListWidgetItem *item) {
    if (!item) return;
    m_appointmentsList->clear();
    int pid = item->data(Qt::UserRole).toInt();
    QList<Appointment> apps = m_dataManager.getPatientAppointments(pid);
    if (apps.isEmpty()) {
        m_appointmentsList->addItem("У этого пациента нет приёмов.");
        return;
    }

    std::sort(apps.begin(), apps.end(), [](const Appointment &a, const Appointment &b) {
        return a.date > b.date;
    });

    for (const auto &ap : apps) {
        Doctor d = m_dataManager.getDoctorById(ap.id_doctor);
        QString line = QString("%1 - Врач: %2 %3")
            .arg(ap.date.toString("dd.MM.yyyy HH:mm"), d.lname, d.fname);
        QListWidgetItem *apItem = new QListWidgetItem(line);
        apItem->setData(Qt::UserRole, ap.id_ap);
        m_appointmentsList->addItem(apItem);
    }
    
    connect(m_appointmentsList, &QListWidget::itemClicked, this, &PatientHistoryWidget::onAppointmentSelected);
}

void PatientHistoryWidget::onAppointmentSelected(QListWidgetItem *item) {
    if (!item) return;
    int appointmentId = item->data(Qt::UserRole).toInt();
    
    Recipe recipe = m_dataManager.getRecipeByAppointmentId(appointmentId);
    Appointment ap = m_dataManager.getAppointmentById(appointmentId);
    Doctor d = m_dataManager.getDoctorById(ap.id_doctor);
    Diagnosis diag = m_dataManager.getDiagnosisById(recipe.id_diagnosis);
    
    QDialog *detailDialog = new QDialog(this);
    detailDialog->setWindowTitle(QString("Детали приёма №%1").arg(appointmentId));
    detailDialog->setMinimumWidth(500);
    detailDialog->setMinimumHeight(400);
    
    QVBoxLayout *layout = new QVBoxLayout(detailDialog);
    
    QLabel *headerLabel = new QLabel(QString("Врач: %1 %2\nДата: %3")
        .arg(d.fname, d.lname, ap.date.toString("dd.MM.yyyy HH:mm")));
    headerLabel->setStyleSheet("font-weight: bold; font-size: 12pt;");
    layout->addWidget(headerLabel);
    
    layout->addWidget(new QLabel("Диагноз:"));
    QTextEdit *diagEdit = new QTextEdit();
    diagEdit->setText(diag.name);
    diagEdit->setReadOnly(true);
    diagEdit->setMaximumHeight(50);
    layout->addWidget(diagEdit);
    
    layout->addWidget(new QLabel("Жалобы:"));
    QTextEdit *complaintsEdit = new QTextEdit();
    complaintsEdit->setText(recipe.complaints);
    complaintsEdit->setReadOnly(true);
    layout->addWidget(complaintsEdit);
    
    layout->addWidget(new QLabel("Рекомендации:"));
    QTextEdit *recsEdit = new QTextEdit();
    recsEdit->setText(recipe.recommendations);
    recsEdit->setReadOnly(true);
    layout->addWidget(recsEdit);
    
    QPushButton *backBtn = new QPushButton("◀ Назад");
    connect(backBtn, &QPushButton::clicked, detailDialog, &QDialog::accept);
    layout->addWidget(backBtn);
    
    detailDialog->exec();
    detailDialog->deleteLater();
}
