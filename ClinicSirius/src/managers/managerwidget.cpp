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

    // Create splitter: sidebar | content
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    
    // Create sidebar
    createSidebar();
    splitter->addWidget(createSidebarWidget());
    
    // Create stacked widget for pages
    m_stack = new QStackedWidget();
    splitter->addWidget(m_stack);
    
    // Set splitter sizes (sidebar narrower, content wider)
    splitter->setSizes({200, 600});
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);

    // Create pages
    createDashboardPage();
    m_schedulePage = new ManagerScheduleViewer(&m_dataManager, this);
    m_roomSchedulePage = new RoomScheduleViewer(this);
    m_patientPage = new PatientManagementDialog(this);
    m_bulkOpsPage = new BulkOperationsDialog(this);
    m_familyPage = new FamilyViewerWidget(this);

    // Add pages to stack
    m_stack->addWidget(m_dashboardPage);      // Index 0
    m_stack->addWidget(m_schedulePage);        // Index 1
    m_stack->addWidget(m_roomSchedulePage);    // Index 2
    m_stack->addWidget(m_patientPage);         // Index 3
    m_stack->addWidget(m_familyPage);          // Index 4
    m_stack->addWidget(m_bulkOpsPage);         // Index 5

    m_stack->setCurrentIndex(DashboardPage);
    
    // Add Esc key shortcut for navigation (except on dashboard)
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

    // User profile section
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
    m_userEmailLabel->setStyleSheet("color: #666;");
    profileLayout->addWidget(m_userEmailLabel);

    layout->addWidget(profileWidget);
    layout->addSpacing(16);

    // Navigation items
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

    // Title
    QLabel* title = new QLabel("Панель менеджера");
    QFont titleFont; titleFont.setPointSize(16); titleFont.setBold(true);
    title->setFont(titleFont);
    main->addWidget(title);

    // Welcome message
    QLabel* welcome = new QLabel("Выберите действие в левом меню для начала работы.");
    welcome->setStyleSheet("color: #666; font-size: 11pt;");
    main->addWidget(welcome);

    main->addSpacing(24);

    // Quick action cards
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(16);

    // Card 1: Schedule
    QWidget* scheduleCard = new QWidget();
    QVBoxLayout* scheduleCardLayout = new QVBoxLayout(scheduleCard);
    scheduleCardLayout->setContentsMargins(16, 16, 16, 16);
    scheduleCard->setStyleSheet("background-color: #f0f4f8; border-radius: 8px;");
    QLabel* scheduleTitle = new QLabel("📅 Управление расписанием");
    QFont cardFont; cardFont.setPointSize(12); cardFont.setBold(true);
    scheduleTitle->setFont(cardFont);
    scheduleCardLayout->addWidget(scheduleTitle);
    QLabel* scheduleDesc = new QLabel("Просмотр и управление расписанием врачей, создание записей.");
    scheduleDesc->setWordWrap(true);
    scheduleDesc->setStyleSheet("color: #666; margin-top: 8px;");
    scheduleCardLayout->addWidget(scheduleDesc);
    cardsLayout->addWidget(scheduleCard);

    // Card 2: Patients
    QWidget* patientsCard = new QWidget();
    QVBoxLayout* patientsCardLayout = new QVBoxLayout(patientsCard);
    patientsCardLayout->setContentsMargins(16, 16, 16, 16);
    patientsCard->setStyleSheet("background-color: #f0f4f8; border-radius: 8px;");
    QLabel* patientsTitle = new QLabel("👥 Управление пациентами");
    patientsTitle->setFont(cardFont);
    patientsCardLayout->addWidget(patientsTitle);
    QLabel* patientsDesc = new QLabel("Добавление, редактирование и управление профилями пациентов.");
    patientsDesc->setWordWrap(true);
    patientsDesc->setStyleSheet("color: #666; margin-top: 8px;");
    patientsCardLayout->addWidget(patientsDesc);
    cardsLayout->addWidget(patientsCard);

    // Card 3: Bulk operations
    QWidget* bulkCard = new QWidget();
    QVBoxLayout* bulkCardLayout = new QVBoxLayout(bulkCard);
    bulkCardLayout->setContentsMargins(16, 16, 16, 16);
    bulkCard->setStyleSheet("background-color: #f0f4f8; border-radius: 8px;");
    QLabel* bulkTitle = new QLabel("🛠 Массовые операции");
    bulkTitle->setFont(cardFont);
    bulkCardLayout->addWidget(bulkTitle);
    QLabel* bulkDesc = new QLabel("Создание слотов расписания, пакетные операции.");
    bulkDesc->setWordWrap(true);
    bulkDesc->setStyleSheet("color: #666; margin-top: 8px;");
    bulkCardLayout->addWidget(bulkDesc);
    cardsLayout->addWidget(bulkCard);

    main->addLayout(cardsLayout);
    main->addStretch();
}

void ManagerWidget::createSidebar() {
    // Sidebar is created in createSidebarWidget()
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
    
    // Load manager info from database
    Manager manager = m_dataManager.getManagerById(user.id);
    m_userNameLabel->setText(manager.fullName().isEmpty() ? "Менеджер" : manager.fullName());
    m_userEmailLabel->setText(manager.email.isEmpty() ? "email@clinic.com" : manager.email);
    
    // Pass user to family page that needs it
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
