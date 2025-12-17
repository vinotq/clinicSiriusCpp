#ifndef MANAGERWIDGET_H
#define MANAGERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QShortcut>
#include "common/datamanager.h"
#include "common/models.h"

class PatientManagementDialog;
class ManagerScheduleViewer;
class RoomScheduleViewer;
class BulkOperationsDialog;
class FamilyViewerWidget;

class ManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit ManagerWidget(QWidget* parent = nullptr);
    void setUser(const LoginUser& user);

signals:
    void requestLogout();

private slots:
    // Sidebar navigation
    void onSidebarItemClicked(int index);
    // Clinic management actions
    void onViewClinicSchedules();
    void onManagePatients();
    void onBulkOperations();
    // Navigation
    void goToDashboard();

private:
    void buildUI();
    void createDashboardPage();
    void createSidebar();
    QWidget* createSidebarWidget();
    void updateSidebarHighlight(int pageIndex);

    DataManager m_dataManager;
    LoginUser m_user;

    // Main layout and stacked pages
    QStackedWidget* m_stack = nullptr;
    QWidget* m_dashboardPage = nullptr;
    QWidget* m_schedulePage = nullptr;
    QWidget* m_roomSchedulePage = nullptr;
    PatientManagementDialog* m_patientPage = nullptr;
    BulkOperationsDialog* m_bulkOpsPage = nullptr;
    FamilyViewerWidget* m_familyPage = nullptr;
    
    // Sidebar components
    QListWidget* m_sidebar = nullptr;
    QLabel* m_userNameLabel = nullptr;
    QLabel* m_userEmailLabel = nullptr;
    
    // Page enum for tracking current page
    enum PageIndex { DashboardPage = 0, SchedulePage = 1, RoomSchedulePage = 2, PatientPage = 3, FamilyPage = 4, BulkOpsPage = 5 };
};

#endif // MANAGERWIDGET_H
