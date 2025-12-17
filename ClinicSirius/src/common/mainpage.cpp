#include "mainpage.h"
#include "profilewidget.h"
#include "doctorwidget.h"
#include "doctorprofilewidget.h"
#include "appointmentbookingwidget.h"
#include "patienthistorywidget.h"
#include "managers/managerwidget.h"
#include "managers/managerprofilewidget.h"
#include "admins/adminwidget.h"
#include "admins/adminprofilewidget.h" // CHANGED: Include admin profile widget

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFont>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <QIcon>
#include <QPixmap>
#include <QSize>

// ServiceCard implementation
ServiceCard::ServiceCard(const QString& title, const QString& description,
                         const QString& iconPath, QWidget *parent)
    : QWidget(parent) {
    setupUI(title, description, iconPath);
    applyStyles();
}

void ServiceCard::setupUI(const QString& title, const QString& description, const QString& iconPath) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    QLabel *iconLabel = new QLabel();
    iconLabel->setProperty("class", "service-icon");
    if (iconPath.startsWith(":/")) {
        if (iconPath.endsWith(".svg", Qt::CaseInsensitive)) {
            QPixmap pm = QIcon(iconPath).pixmap(QSize(36,36));
            if (!pm.isNull()) iconLabel->setPixmap(pm);
        } else {
            QPixmap pix(iconPath);
            if (!pix.isNull()) iconLabel->setPixmap(pix.scaledToHeight(36, Qt::SmoothTransformation));
        }
    } else if (!iconPath.isEmpty()) {
        iconLabel->setText(iconPath);
        QFont iconFont; iconFont.setPointSize(36); iconLabel->setFont(iconFont);
    }
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    QLabel *titleLabel = new QLabel(title);
    QFont titleFont; titleFont.setPointSize(13); titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);

    QLabel *descriptionLabel = new QLabel(description);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setProperty("class", "service-card-description");
    layout->addWidget(descriptionLabel);

    layout->addStretch();
    setMinimumHeight(180);
}

void ServiceCard::applyStyles() { setProperty("class", "service-card"); }

// ========== MainPage ===========

MainPage::MainPage(QWidget *parent)
    : QWidget(parent) {
    setupUI();
    applyStyles();
}

MainPage::~MainPage() {}

void MainPage::setupUI() {
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(40, 20, 40, 20);
    headerLayout->setSpacing(20);

    logoButton = new QPushButton();
    logoButton->setFlat(true);
    QIcon clinicIcon(":/images/clinic.svg");
    logoButton->setIcon(clinicIcon);
    logoButton->setIconSize(QSize(48,48));
    logoButton->setText("Клиника «Сириус»");
    QFont logoFont; logoFont.setPointSize(16); logoButton->setFont(logoFont);
    logoButton->setProperty("class", "header-logo-btn");

    headerLayout->addWidget(logoButton);
    headerLayout->addStretch();

    buildHeader(headerLayout);

    QWidget *headerWidget = new QWidget();
    headerWidget->setLayout(headerLayout);
    headerWidget->setProperty("class", "header-widget");
    mainLayout->addWidget(headerWidget);

    // Build landing page content first
    buildLanding();

    // Content stack
    contentStack = new QStackedWidget(this);
    mainLayout->addWidget(contentStack);

    // Instantiate widgets
    profileWidget = new ProfileWidget(this);
    connect(profileWidget, &ProfileWidget::requestLogout, this, &MainPage::logoutRequested);
    connect(profileWidget, &ProfileWidget::requestAccountDeletion, this, &MainPage::logoutRequested);

    appointmentBookingWidget = new AppointmentBookingWidget(this);
    patientHistoryWidget = new PatientHistoryWidget(this);
    managerWidget = new ManagerWidget(this);
    managerProfileWidget = new ManagerProfileWidget(this);
    adminWidget = new AdminWidget(this);
    adminProfileWidget = new AdminProfileWidget(this); // CHANGED: Create admin profile widget
    connect(adminProfileWidget, &AdminProfileWidget::requestLogout, this, &MainPage::logoutRequested); // CHANGED: Connect admin profile logout signal
    doctorProfileWidget = new DoctorProfileWidget(this);
    doctorWidget = new DoctorWidget(this);

    // Add widgets to stack
    contentStack->addWidget(landingPage);
    contentStack->addWidget(profileWidget);
    contentStack->addWidget(appointmentBookingWidget);
    contentStack->addWidget(patientHistoryWidget);
    contentStack->addWidget(managerWidget);
    contentStack->addWidget(managerProfileWidget);
    contentStack->addWidget(adminWidget);
    contentStack->addWidget(adminProfileWidget); // CHANGED: Add admin profile widget to stack
    contentStack->addWidget(doctorProfileWidget);

    buildDoctorLanding();
    contentStack->addWidget(doctorLandingPage);
    contentStack->addWidget(doctorWidget);

    connect(this, &MainPage::navigateToBooking, this, &MainPage::showBooking);

    // Logo navigation
    connect(logoButton, &QPushButton::clicked, this, [this]() {
        switch (currentUser.type) {
            case LoginUser::PATIENT:
                if (landingPage) contentStack->setCurrentWidget(landingPage);
                break;
            case LoginUser::DOCTOR:
                if (doctorLandingPage) contentStack->setCurrentWidget(doctorLandingPage);
                break;
            case LoginUser::MANAGER:
                if (managerWidget) contentStack->setCurrentWidget(managerWidget);
                break;
            case LoginUser::ADMIN:
                if (adminWidget) contentStack->setCurrentWidget(adminWidget);
                break;
            default:
                if (landingPage) contentStack->setCurrentWidget(landingPage);
                break;
        }
    });
}

void MainPage::applyStyles() {
    setProperty("class", "main-page");
}

void MainPage::adjustHeaderForRole() {
    // Ensure menu actions exist
    if (!userMenu) return;

    // Default: enable profile/settings
    profileAction->setVisible(true);
    settingsAction->setVisible(true);

    // Staff members (Admin, Manager, Doctor) do not need a separate profile entry
    // They access their info via settings instead
    if (currentUser.type == LoginUser::ADMIN || currentUser.type == LoginUser::MANAGER || currentUser.type == LoginUser::DOCTOR) {
        profileAction->setVisible(false);
    }

    // Update the displayed name on the menu button
    QString nameText = currentUser.name.isEmpty() ? (currentUser.type == LoginUser::ADMIN ? "Администратор" : "Профиль") : currentUser.name;
    userMenuButton->setText(nameText);
}

void MainPage::buildHeader(QHBoxLayout *headerLayout) {
    // Header: booking button removed — moved to doctor's schedule UI

    userMenuButton = new QToolButton();
    userMenuButton->setPopupMode(QToolButton::InstantPopup);
    userMenuButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    userMenuButton->setText("Профиль");
    userMenuButton->setMinimumHeight(40);
    userMenuButton->setProperty("class", "header-profile-btn");

    userMenu = new QMenu(userMenuButton);
    profileAction = userMenu->addAction("Профиль");
    profileAction->setText(QString::fromUtf8("👤 ") + profileAction->text());
    settingsAction = userMenu->addAction("Настройки");
    settingsAction->setText(QString::fromUtf8("⚙️ ") + settingsAction->text());
    userMenu->addSeparator();
    logoutAction = userMenu->addAction("Выход");
    logoutAction->setText(QString::fromUtf8("🚪 ") + logoutAction->text());

    connect(profileAction, &QAction::triggered, this, [this]() { 
        if (currentUser.type == LoginUser::DOCTOR) {
            showDoctorProfile(false);
        } else {
            // Managers don't have a separate profile widget in the header;
            // route managers to the manager dashboard instead
            if (currentUser.type == LoginUser::MANAGER && managerWidget) {
                managerWidget->setUser(currentUser);
                contentStack->setCurrentWidget(managerWidget);
            } else {
                showProfile(false);
            }
        }
    });
    connect(settingsAction, &QAction::triggered, this, [this]() { 
        if (currentUser.type == LoginUser::DOCTOR) {
            showDoctorProfile(true);
        } else if (currentUser.type == LoginUser::MANAGER) {
            // Managers: show manager profile widget as settings (no separate profile)
            if (managerProfileWidget) {
                managerProfileWidget->setUser(currentUser);
                contentStack->setCurrentWidget(managerProfileWidget);
            }
        } else if (currentUser.type == LoginUser::ADMIN) {
            // CHANGED: Admins: show admin profile widget as settings (similar to manager)
            if (adminProfileWidget) {
                adminProfileWidget->setUser(currentUser);
                contentStack->setCurrentWidget(adminProfileWidget);
            }
        } else {
            showProfile(true);
        }
    });
    connect(logoutAction, &QAction::triggered, this, &MainPage::logoutRequested);

    userMenuButton->setMenu(userMenu);
    headerLayout->addWidget(userMenuButton);
}

void MainPage::buildLanding() {
    QWidget *contentWidget = new QWidget();
    contentWidget->setProperty("class", "content-widget");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(60, 30, 60, 30);
    contentLayout->setSpacing(8);

    QWidget *heroSection = new QWidget();
    heroSection->setProperty("class", "hero-section-widget");
    QVBoxLayout *heroLayout = new QVBoxLayout(heroSection);
    heroLayout->setContentsMargins(30, 25, 30, 25);
    heroLayout->setSpacing(12);

    welcomeLabel = new QLabel("Добро пожаловать в клинику «Сириус»");
    QFont welcomeFont;
    welcomeFont.setPointSize(24);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setProperty("class", "welcome-label");
    heroLayout->addWidget(welcomeLabel);

    descriptionLabel = new QLabel(
        "Современная клиника с полным спектром медицинских услуг. "
        "Запишитесь на прием онлайн и получите консультацию у лучших врачей."
    );
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setProperty("class", "description-label");
    heroLayout->addWidget(descriptionLabel);

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(15);

    QPushButton *profileActionButton = new QPushButton("👤 Личный кабинет");
    profileActionButton->setMinimumHeight(45);
    profileActionButton->setMinimumWidth(220);
    profileActionButton->setProperty("class", "hero-profile-btn");
    connect(profileActionButton, &QPushButton::clicked, this, [this]() { showProfile(false); });
    actionsLayout->addWidget(profileActionButton);

    QPushButton *bookingActionButton = new QPushButton("✅ Записать прием");
    bookingActionButton->setMinimumHeight(45);
    bookingActionButton->setMinimumWidth(220);
    bookingActionButton->setProperty("class", "hero-booking-btn");
    connect(bookingActionButton, &QPushButton::clicked, this, &MainPage::navigateToBooking);
    actionsLayout->addWidget(bookingActionButton);

    actionsLayout->addStretch();
    heroLayout->addLayout(actionsLayout);
    contentLayout->addWidget(heroSection);
    contentLayout->addSpacing(24);

    QLabel *servicesTitle = new QLabel("Наши услуги");
    QFont servicesTitleFont;
    servicesTitleFont.setPointSize(18);
    servicesTitleFont.setBold(true);
    servicesTitle->setFont(servicesTitleFont);
    servicesTitle->setProperty("class", "services-title");
    contentLayout->addWidget(servicesTitle);

    QGridLayout *servicesLayout = new QGridLayout();
    servicesLayout->setSpacing(18);
    servicesLayout->setContentsMargins(0, 0, 0, 0);

    ServiceCard *onlineBookingCard = new ServiceCard(
        "Онлайн запись",
        "Запишитесь на прием в врачу в удобное время прямо из личного кабинета",
        ":/images/clinic.svg"
    );
    servicesLayout->addWidget(onlineBookingCard, 0, 0);

    ServiceCard *doctorsCard = new ServiceCard(
        "Квалифицированные врачи",
        "Лучше специалисты различных направлений готовы помочь вам",
        ":/images/doctor.svg"
    );
    servicesLayout->addWidget(doctorsCard, 0, 1);

    ServiceCard *confidentialityCard = new ServiceCard(
        "Конфиденциальность",
        "Все данные пациентов защищены и хранятся в безопасности",
        ":/images/clinic.svg"
    );
    servicesLayout->addWidget(confidentialityCard, 0, 2);

    contentLayout->addLayout(servicesLayout);
    contentLayout->addSpacing(24);

    QLabel *featureTitle = new QLabel("Преимущества нашей клиники");
    QFont featureTitleFont;
    featureTitleFont.setPointSize(18);
    featureTitleFont.setBold(true);
    featureTitle->setFont(featureTitleFont);
    featureTitle->setProperty("class", "services-title");
    contentLayout->addWidget(featureTitle);

    QGridLayout *featuresLayout = new QGridLayout();
    featuresLayout->setSpacing(15);
    featuresLayout->setContentsMargins(0, 0, 0, 0);

    QStringList features = {
        "Удобное расписание — возможность выбрать удобное время приема",
        "Быстрое обслуживание — современное оборудование и опытный персонал",
        "Семейные услуги — прием для всех членов семьи",
        "Справки и документы — оформление необходимых документов"
    };

    int row = 0, col = 0;
    for (const QString& feature : features) {
        QLabel *featureLabel = new QLabel(feature);
        featureLabel->setWordWrap(true);
        featureLabel->setProperty("class", "feature-label");
        featuresLayout->addWidget(featureLabel, row, col);

        col++;
        if (col == 2) {
            col = 0;
            row++;
        }
    }

    contentLayout->addLayout(featuresLayout);
    contentLayout->addSpacing(24);

    QLabel *faqTitle = new QLabel("Часто задаваемые вопросы");
    QFont faqTitleFont;
    faqTitleFont.setPointSize(18);
    faqTitleFont.setBold(true);
    faqTitle->setFont(faqTitleFont);
    faqTitle->setProperty("class", "services-title");
    contentLayout->addWidget(faqTitle);

    QStringList questions = {
        "Как записаться на прием?",
        "Какие документы нужны при визите?",
        "Как отменить или перенести прием?",
        "Как добавить членов семьи?"
    };

    QStringList answers = {
        "Зарегистрируйтесь в системе, заполните личные данные и выберите удобное время приема из доступных слотов. Запись будет подтверждена автоматически.",
        "При первом визите обязательно иметь паспорт, полис ОМС и СНИЛС. Дополнительно могут потребоваться другие документы в зависимости от причины посещения.",
        "Отмену или перенос приема можно выполнить через личный кабинет за 24 часа до приема. В экстренных случаях позвоните в клинику.",
        "В разделе 'Моя семья' добавьте данные членов семьи и выберите их как пациентов при записи на прием."
    };

    for (int i = 0; i < questions.size(); ++i) {
        QLabel *questionLabel = new QLabel(questions[i]);
        QFont questionFont;
        questionFont.setPointSize(13);
        questionFont.setBold(true);
        questionLabel->setFont(questionFont);
        questionLabel->setProperty("class", "faq-question-label");
        contentLayout->addWidget(questionLabel);

        QLabel *answerLabel = new QLabel(answers[i]);
        answerLabel->setWordWrap(true);
        answerLabel->setProperty("class", "faq-answer-label");
        contentLayout->addWidget(answerLabel);
    }

    contentLayout->addSpacing(24);

    QLabel *contactsTitle = new QLabel("Контакты");
    QFont contactsTitleFont;
    contactsTitleFont.setPointSize(18);
    contactsTitleFont.setBold(true);
    contactsTitle->setFont(contactsTitleFont);
    contactsTitle->setAlignment(Qt::AlignCenter);
    contactsTitle->setProperty("class", "services-title");
    contentLayout->addWidget(contactsTitle);

    contentLayout->addSpacing(12);

    QHBoxLayout *contactsLayout = new QHBoxLayout();
    contactsLayout->setSpacing(30);

    QVBoxLayout *phoneLayout = new QVBoxLayout();
    QLabel *phoneIconLabel = new QLabel("📞");
    QFont pi; pi.setPointSize(20); phoneIconLabel->setFont(pi);
    phoneIconLabel->setAlignment(Qt::AlignCenter);
    phoneLayout->addWidget(phoneIconLabel);

    QLabel *phoneLabel = new QLabel("Телефон");
    QFont phoneFont;
    phoneFont.setPointSize(14);
    phoneFont.setBold(true);
    phoneLabel->setFont(phoneFont);
    phoneLabel->setAlignment(Qt::AlignCenter);
    phoneLabel->setProperty("class", "contact-label");
    phoneLayout->addWidget(phoneLabel);

    QLabel *phoneNumberLabel = new QLabel("8 (862) 444-01-03");
    QFont phoneNumberFont;
    phoneNumberFont.setPointSize(12);
    phoneNumberLabel->setFont(phoneNumberFont);
    phoneNumberLabel->setAlignment(Qt::AlignCenter);
    phoneNumberLabel->setProperty("class", "contact-value-label");
    phoneLayout->addWidget(phoneNumberLabel);

    QVBoxLayout *addressLayout = new QVBoxLayout();
    QLabel *addressIconLabel = new QLabel("📍");
    QFont ai; ai.setPointSize(20); addressIconLabel->setFont(ai);
    addressIconLabel->setAlignment(Qt::AlignCenter);
    addressLayout->addWidget(addressIconLabel);

    QLabel *addressLabel = new QLabel("Адрес");
    QFont addressFont;
    addressFont.setPointSize(14);
    addressFont.setBold(true);
    addressLabel->setFont(addressFont);
    addressLabel->setAlignment(Qt::AlignCenter);
    addressLabel->setProperty("class", "contact-label");
    addressLayout->addWidget(addressLabel);

    QLabel *addressValueLabel = new QLabel("пгт. Сириус, ул. Старообрядческая, д. 64");
    QFont addressValueFont;
    addressValueFont.setPointSize(12);
    addressValueLabel->setFont(addressValueFont);
    addressValueLabel->setAlignment(Qt::AlignCenter);
    addressValueLabel->setProperty("class", "contact-value-label");
    addressLayout->addWidget(addressValueLabel);

    QVBoxLayout *hoursLayout = new QVBoxLayout();
    QLabel *hoursIconLabel = new QLabel("⏰");
    QFont hi; hi.setPointSize(20); hoursIconLabel->setFont(hi);
    hoursIconLabel->setAlignment(Qt::AlignCenter);
    hoursLayout->addWidget(hoursIconLabel);

    QLabel *hoursLabel = new QLabel("Часы работы");
    QFont hoursFont;
    hoursFont.setPointSize(14);
    hoursFont.setBold(true);
    hoursLabel->setFont(hoursFont);
    hoursLabel->setAlignment(Qt::AlignCenter);
    hoursLabel->setProperty("class", "contact-label");
    hoursLayout->addWidget(hoursLabel);

    QLabel *hoursValueLabel = new QLabel("Пн-Пт: 08:00 - 20:00\nСб-Вс: 08:00 - 15:00");
    QFont hoursValueFont;
    hoursValueFont.setPointSize(12);
    hoursValueLabel->setFont(hoursValueFont);
    hoursValueLabel->setAlignment(Qt::AlignCenter);
    hoursValueLabel->setProperty("class", "contact-value-label");
    hoursLayout->addWidget(hoursValueLabel);

    contactsLayout->addLayout(phoneLayout);
    contactsLayout->addLayout(addressLayout);
    contactsLayout->addLayout(hoursLayout);

    contentLayout->addLayout(contactsLayout);
    contentLayout->addSpacing(20);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);

    landingPage = new QWidget();
    QVBoxLayout *wrapperLayout = new QVBoxLayout(landingPage);
    wrapperLayout->setContentsMargins(0, 0, 0, 0);
    wrapperLayout->addWidget(scrollArea);
    landingPage->setProperty("class", "content-wrapper");

    setMinimumSize(1000, 700);
}

void MainPage::buildDoctorLanding() {
    doctorLandingPage = new QWidget();
    doctorLandingPage->setProperty("class", "content-wrapper");
    QVBoxLayout *mainLayout = new QVBoxLayout(doctorLandingPage);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *contentWidget = new QWidget();
    contentWidget->setProperty("class", "content-widget");
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(60, 30, 60, 30);
    contentLayout->setSpacing(8);

    // Hero section with welcome message
    QWidget *heroSection = new QWidget();
    heroSection->setProperty("class", "hero-section-widget");
    QVBoxLayout *heroLayout = new QVBoxLayout(heroSection);
    heroLayout->setContentsMargins(30, 25, 30, 25);
    heroLayout->setSpacing(12);

    QLabel *welcomeLabel = new QLabel("Добро пожаловать в кабинет врача");
    QFont welcomeFont;
    welcomeFont.setPointSize(24);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    welcomeLabel->setProperty("class", "welcome-label");
    heroLayout->addWidget(welcomeLabel);

    QLabel *descriptionLabel = new QLabel(
        "Управляйте своим расписанием, просматривайте назначения пациентов и ведите их медицинские записи."
    );
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setProperty("class", "description-label");
    heroLayout->addWidget(descriptionLabel);

    // Action buttons
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(15);

        QPushButton *scheduleButton = new QPushButton(QString::fromUtf8("👩\u200D⚕️ ") + "Расписание");
    scheduleButton->setMinimumHeight(45);
    scheduleButton->setMinimumWidth(220);
    scheduleButton->setProperty("class", "hero-booking-btn");
    connect(scheduleButton, &QPushButton::clicked, this, [this]() {
        contentStack->setCurrentWidget(doctorWidget);
    });
    actionsLayout->addWidget(scheduleButton);

    QPushButton *historyButton = new QPushButton(QString::fromUtf8("🕘 ") + "История приёмов пациента");
    historyButton->setMinimumHeight(45);
    historyButton->setMinimumWidth(220);
    historyButton->setProperty("class", "hero-history-btn");
    connect(historyButton, &QPushButton::clicked, this, [this]() {
        contentStack->setCurrentWidget(patientHistoryWidget);
    });
    actionsLayout->addWidget(historyButton);

    actionsLayout->addStretch();
    heroLayout->addLayout(actionsLayout);
    contentLayout->addWidget(heroSection);
    contentLayout->addStretch();

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidget(contentWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setProperty("class", "no-border-scrollarea");

    mainLayout->addWidget(scrollArea);
}
void MainPage::setCurrentUser(const LoginUser &user) {
    currentUser = user;
    updateUserChip();
    profileWidget->setUser(user);
    doctorProfileWidget->setUser(user);
    doctorWidget->setUser(user);
    appointmentBookingWidget->setUser(user);
    
    // adjust header/menu according to role
    adjustHeaderForRole();

    // For doctors and managers show role-specific landing
    if (user.type == LoginUser::DOCTOR) {
        contentStack->setCurrentWidget(doctorLandingPage);
    } else if (user.type == LoginUser::MANAGER) {
        managerWidget->setUser(user);
        managerProfileWidget->setUser(user);
        contentStack->setCurrentWidget(managerWidget);
    } else if (user.type == LoginUser::ADMIN) {
        adminWidget->setUser(user);
        contentStack->setCurrentWidget(adminWidget);
    } else {
        showHome();
    }
}

void MainPage::updateUserChip() {
    QString nameText = currentUser.name.isEmpty() ? "Профиль" : currentUser.name;
    userMenuButton->setText(nameText);
}

void MainPage::showProfile(bool openSettingsTab) {
    if (openSettingsTab) {
        profileWidget->openSettingsTab();
    }
    contentStack->setCurrentWidget(profileWidget);
}

void MainPage::showDoctorProfile(bool openSettingsTab) {
    if (openSettingsTab) {
        doctorProfileWidget->openSettingsTab();
    }
    contentStack->setCurrentWidget(doctorProfileWidget);
}

void MainPage::showHome() {
    contentStack->setCurrentWidget(landingPage);
}

void MainPage::showBooking() {
    contentStack->setCurrentWidget(appointmentBookingWidget);
}