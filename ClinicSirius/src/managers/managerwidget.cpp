#include "managers/managerwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QMessageBox>
#include <QShortcut>
#include <QStyledItemDelegate>
#include "managers/managerscheduleviewer.h"
#include "managers/roomscheduleviewer.h"
#include "managers/patientmanagementdialog.h"
#include "managers/bulkoperationsdialog.h"
#include "patients/familyviewerwidget.h"

ManagerWidget::ManagerWidget(QWidget* parent)
    : QWidget(parent), m_dataManager(QString()) {
    buildUI();
}

void ManagerWidget::buildUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    
    createSidebar();
    splitter->addWidget(createSidebarWidget());
    
    m_stack = new QStackedWidget();
    splitter->addWidget(m_stack);
    
    splitter->setSizes({200, 600});
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);

    createDashboardPage();
    m_schedulePage = new ManagerScheduleViewer(&m_dataManager, this);
    m_roomSchedulePage = new RoomScheduleViewer(this);
    m_patientPage = new PatientManagementDialog(this);
    m_bulkOpsPage = new BulkOperationsDialog(this);
    m_familyPage = new FamilyViewerWidget(this);

    m_stack->addWidget(m_dashboardPage);
    m_stack->addWidget(m_schedulePage);
    m_stack->addWidget(m_roomSchedulePage);
    m_stack->addWidget(m_patientPage);
    m_stack->addWidget(m_familyPage);
    m_stack->addWidget(m_bulkOpsPage);

    m_stack->setCurrentIndex(DashboardPage);
    
    new QShortcut(Qt::Key_Escape, this, [this]() {
        if (m_stack->currentIndex() != DashboardPage) {
            goToDashboard();
        }
    });
}

QWidget* ManagerWidget::createSidebarWidget() {
    QWidget* sidebarWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(sidebarWidget);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    QWidget* profileWidget = new QWidget();
    QVBoxLayout* profileLayout = new QVBoxLayout(profileWidget);
    profileLayout->setContentsMargins(0, 0, 0, 0);
    profileLayout->setSpacing(4);

    m_userNameLabel = new QLabel("Менеджер");
    QFont nameFont; nameFont.setPointSize(10); nameFont.setBold(true);
    m_userNameLabel->setFont(nameFont);
    profileLayout->addWidget(m_userNameLabel);

    m_userEmailLabel = new QLabel("email@clinic.com");
    QFont emailFont; emailFont.setPointSize(9);
    m_userEmailLabel->setFont(emailFont);
    m_userEmailLabel->setProperty("class", "manager-email-label");
    profileLayout->addWidget(m_userEmailLabel);

    layout->addWidget(profileWidget);
    layout->addSpacing(16);

    m_sidebar = new QListWidget();
    m_sidebar->setObjectName("navigationSidebar");
    m_sidebar->addItem("📊 Дашборд");
    m_sidebar->addItem("📅 Расписание врачей");
    m_sidebar->addItem("🏥 Расписание кабинетов");
    m_sidebar->addItem("👥 Пациенты");
    m_sidebar->addItem("👨‍👩‍👧 Управление семьей");
    m_sidebar->addItem("🛠 Массовые операции");
    
    m_sidebar->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sidebar->setItemDelegate(new QStyledItemDelegate(m_sidebar));
    m_sidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sidebar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_sidebar->item(0)->setSelected(true);

    connect(m_sidebar, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        int index = m_sidebar->row(item);
        onSidebarItemClicked(index);
    });

    layout->addWidget(m_sidebar, 1);
    layout->addStretch();

    return sidebarWidget;
}

void ManagerWidget::createDashboardPage() {
    m_dashboardPage = new QWidget();
    QVBoxLayout* main = new QVBoxLayout(m_dashboardPage);
    main->setContentsMargins(24, 24, 24, 24);
    main->setSpacing(16);

    QLabel* title = new QLabel("Панель менеджера");
    QFont titleFont; titleFont.setPointSize(16); titleFont.setBold(true);
    title->setFont(titleFont);
    main->addWidget(title);

    QLabel* welcome = new QLabel("Выберите действие в левом меню для начала работы.");
    welcome->setProperty("class", "manager-welcome-label");
    main->addWidget(welcome);

    main->addSpacing(24);

    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(16);

    QWidget* scheduleCard = new QWidget();
    QVBoxLayout* scheduleCardLayout = new QVBoxLayout(scheduleCard);
    scheduleCardLayout->setContentsMargins(16, 16, 16, 16);
    scheduleCard->setProperty("class", "manager-quick-card");
    QLabel* scheduleTitle = new QLabel("📅 Управление расписанием");
    QFont cardFont; cardFont.setPointSize(12); cardFont.setBold(true);
    scheduleTitle->setFont(cardFont);
    scheduleCardLayout->addWidget(scheduleTitle);
    QLabel* scheduleDesc = new QLabel("Просмотр и управление расписанием врачей, создание записей.");
    scheduleDesc->setWordWrap(true);
    scheduleDesc->setProperty("class", "manager-quick-desc");
    scheduleCardLayout->addWidget(scheduleDesc);
    cardsLayout->addWidget(scheduleCard);

    QWidget* patientsCard = new QWidget();
    QVBoxLayout* patientsCardLayout = new QVBoxLayout(patientsCard);
    patientsCardLayout->setContentsMargins(16, 16, 16, 16);
    patientsCard->setProperty("class", "manager-quick-card");
    QLabel* patientsTitle = new QLabel("👥 Управление пациентами");
    patientsTitle->setFont(cardFont);
    patientsCardLayout->addWidget(patientsTitle);
    QLabel* patientsDesc = new QLabel("Добавление, редактирование и управление профилями пациентов.");
    patientsDesc->setWordWrap(true);
    patientsDesc->setProperty("class", "manager-quick-desc");
    patientsCardLayout->addWidget(patientsDesc);
    cardsLayout->addWidget(patientsCard);

    QWidget* bulkCard = new QWidget();
    QVBoxLayout* bulkCardLayout = new QVBoxLayout(bulkCard);
    bulkCardLayout->setContentsMargins(16, 16, 16, 16);
    bulkCard->setProperty("class", "manager-quick-card");
    QLabel* bulkTitle = new QLabel("🛠 Массовые операции");
    bulkTitle->setFont(cardFont);
    bulkCardLayout->addWidget(bulkTitle);
    QLabel* bulkDesc = new QLabel("Создание слотов расписания, пакетные операции.");
    bulkDesc->setWordWrap(true);
    bulkDesc->setProperty("class", "manager-quick-desc");
    bulkCardLayout->addWidget(bulkDesc);
    cardsLayout->addWidget(bulkCard);

    main->addLayout(cardsLayout);
    main->addStretch();
}

void ManagerWidget::createSidebar() {
}

void ManagerWidget::onSidebarItemClicked(int index) {
    m_stack->setCurrentIndex(index);
    updateSidebarHighlight(index);
}

void ManagerWidget::updateSidebarHighlight(int pageIndex) {
    for (int i = 0; i < m_sidebar->count(); ++i) {
        m_sidebar->item(i)->setSelected(i == pageIndex);
    }
}

void ManagerWidget::setUser(const LoginUser& user) {
    m_user = user;
    if (!m_userNameLabel || !m_userEmailLabel) return;
    
    Manager manager = m_dataManager.getManagerById(user.id);
    m_userNameLabel->setText(manager.fullName().isEmpty() ? "Менеджер" : manager.fullName());
    m_userEmailLabel->setText(manager.email.isEmpty() ? "email@clinic.com" : manager.email);
    
    if (m_familyPage) {
        m_familyPage->setUser(user);
    }
}

void ManagerWidget::onViewClinicSchedules() {
    if (m_stack) {
        m_stack->setCurrentIndex(SchedulePage);
        updateSidebarHighlight(SchedulePage);
    }
}

void ManagerWidget::onManagePatients() {
    if (m_stack) {
        m_stack->setCurrentIndex(PatientPage);
        updateSidebarHighlight(PatientPage);
    }
}

void ManagerWidget::onBulkOperations() {
    if (m_stack) {
        m_stack->setCurrentIndex(BulkOpsPage);
        updateSidebarHighlight(BulkOpsPage);
    }
}

void ManagerWidget::goToDashboard() {
    if (m_stack) {
        m_stack->setCurrentIndex(DashboardPage);
        updateSidebarHighlight(DashboardPage);
    }
}
