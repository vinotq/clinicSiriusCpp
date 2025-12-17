#include "profilewidget.h"
#include "addbycodedialog.h"
#include "createpatientdialog.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QMessageBox>
#include <QDate>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QSet>
#include <QJsonArray>
#include <QStyle>
#include <QIcon>
#include <QPixmap>
#include <QSize>

ProfileWidget::ProfileWidget(QWidget *parent)
    : QWidget(parent),
      dataManager(QCoreApplication::applicationDirPath() + "/../data"),
      familyCountBadge(nullptr) {
    buildUI();
}

void ProfileWidget::buildUI() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Основной контент с табами
    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    tabs = new QTabWidget(this);
    
    buildProfileTab();
    buildSettingsTab();

    contentLayout->addWidget(tabs);
    mainLayout->addLayout(contentLayout);

    connectSignals();
}

void ProfileWidget::buildProfileTab() {
    QWidget *profileTab = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profileTab);
    profileLayout->setContentsMargins(0, 0, 0, 0);
    profileLayout->setSpacing(0);

    // Создаем прокручиваемый контент
    ScrollableTab *scrollable = new ScrollableTab();

    // ===== Информация пользователя - Карточка =====
    QWidget *infoCard = new QWidget();
    infoCard->setProperty("class", "modern-card");
    infoCard->setMinimumHeight(180);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoCard);
    infoLayout->setContentsMargins(20, 16, 20, 16);
    infoLayout->setSpacing(10);

    // Заголовок карточки
    QLabel *profileTitle = new QLabel("Основная информация");
    profileTitle->setProperty("class", "info-title");
    infoLayout->addWidget(profileTitle);

    // Сетка с информацией
    QWidget *infoGrid = new QWidget();
    QHBoxLayout *gridLayout = new QHBoxLayout(infoGrid);
    gridLayout->setSpacing(24);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    // Левая часть
    QVBoxLayout *leftCol = new QVBoxLayout();
    leftCol->setSpacing(8);

    nameValue = new QLabel("—");
    nameValue->setProperty("class", "profile-value-main");
    emailValue = new QLabel("—");
    emailValue->setProperty("class", "profile-value-secondary");
    
    leftCol->addWidget(new QLabel("ФИО:"));
    leftCol->addWidget(nameValue);
    leftCol->addSpacing(8);
    leftCol->addWidget(new QLabel("Email:"));
    leftCol->addWidget(emailValue);

    // Правая часть
    QVBoxLayout *rightCol = new QVBoxLayout();
    rightCol->setSpacing(8);

    phoneValue = new QLabel("—");
    phoneValue->setProperty("class", "profile-value-secondary");
    birthValue = new QLabel("—");
    birthValue->setProperty("class", "profile-value-secondary");

    rightCol->addWidget(new QLabel("Телефон:"));
    rightCol->addWidget(phoneValue);
    rightCol->addSpacing(8);
    rightCol->addWidget(new QLabel("Дата рождения:"));
    rightCol->addWidget(birthValue);

    // Дополнительная колонна с СНИЛС/ОМС
    QVBoxLayout *extraCol = new QVBoxLayout();
    extraCol->setSpacing(8);

    snilsValue = new QLabel("***");
    snilsValue->setProperty("class", "profile-value-secondary");
    omsValue = new QLabel("***");
    omsValue->setProperty("class", "profile-value-secondary");

    QHBoxLayout *snilsRow = new QHBoxLayout();
    snilsRow->setSpacing(8);
    snilsRow->setContentsMargins(0, 0, 0, 0);
    QLabel *snilsLabel = new QLabel("СНИЛС:");
    QPushButton *snilsShowBtn = new QPushButton();
    snilsShowBtn->setCheckable(true);
    snilsShowBtn->setIcon(QIcon(":/images/icon-eye.svg"));
    snilsShowBtn->setIconSize(QSize(16, 16));
    snilsShowBtn->setMaximumWidth(32);
    snilsShowBtn->setMaximumHeight(24);
    snilsShowBtn->setProperty("class", "icon-button");
    snilsRow->addWidget(snilsLabel);
    snilsRow->addStretch();
    snilsRow->addWidget(snilsShowBtn);

    extraCol->addLayout(snilsRow);
    extraCol->addWidget(snilsValue);
    extraCol->addSpacing(8);

    QHBoxLayout *omsRow = new QHBoxLayout();
    omsRow->setSpacing(8);
    omsRow->setContentsMargins(0, 0, 0, 0);
    QLabel *omsLabel = new QLabel("Полис ОМС:");
    QPushButton *omsShowBtn = new QPushButton();
    omsShowBtn->setCheckable(true);
    omsShowBtn->setIcon(QIcon(":/images/icon-eye.svg"));
    omsShowBtn->setIconSize(QSize(16, 16));
    omsShowBtn->setMaximumWidth(32);
    omsShowBtn->setMaximumHeight(24);
    omsShowBtn->setProperty("class", "icon-button");
    omsRow->addWidget(omsLabel);
    omsRow->addStretch();
    omsRow->addWidget(omsShowBtn);

    extraCol->addLayout(omsRow);
    extraCol->addWidget(omsValue);

    // Логика показа/скрытия СНИЛС
    connect(snilsShowBtn, &QPushButton::toggled, this, [this, snilsShowBtn](bool checked) {
            Patient p = dataManager.getPatientById(currentUser.id);
        snilsValue->setText(checked ? p.snils : "***");
        snilsShowBtn->setIcon(QIcon(checked ? ":/images/icon-eye-off.svg" : ":/images/icon-eye.svg"));
    });

    // Логика показа/скрытия ОМС
    connect(omsShowBtn, &QPushButton::toggled, this, [this, omsShowBtn](bool checked) {
            Patient p = dataManager.getPatientById(currentUser.id);
        omsValue->setText(checked ? p.oms : "***");
        omsShowBtn->setIcon(QIcon(checked ? ":/images/icon-eye-off.svg" : ":/images/icon-eye.svg"));
    });

    gridLayout->addLayout(leftCol);
    gridLayout->addLayout(rightCol);
    gridLayout->addLayout(extraCol);
    gridLayout->addStretch();
    gridLayout->setStretch(0, 1);
    gridLayout->setStretch(1, 1);
    gridLayout->setStretch(2, 1);

    infoLayout->addWidget(infoGrid);
    scrollable->addContent(infoCard);

    // ===== Статистика Семьи - Бейджи (только кол-во членов) =====
    QWidget *statsContainer = new QWidget();
    QHBoxLayout *statsLayout = new QHBoxLayout(statsContainer);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(16);

    familyCountBadge = new StatsBadge();
    familyCountBadge->setLabel("Членов семьи");
    familyCountBadge->setValue(0);
    familyCountBadge->setBadgeType(StatsBadge::Primary);
    familyCountBadge->setIcon(":/images/icon-user.svg");

    statsLayout->addWidget(familyCountBadge);
    statsLayout->addStretch();

    scrollable->addContent(statsContainer);

    // ===== Семья - Список =====
    QWidget *familyCard = new QWidget();
    familyCard->setProperty("class", "modern-card");
    QVBoxLayout *familyLayout = new QVBoxLayout(familyCard);
    familyLayout->setContentsMargins(20, 16, 20, 16);
    familyLayout->setSpacing(12);

    QLabel *familyTitle = new QLabel("Члены семьи");
    familyTitle->setProperty("class", "section-title");
    familyLayout->addWidget(familyTitle);

    // Поиск в семье
    familySearchBox = new QLineEdit();
    familySearchBox->setPlaceholderText("Поиск по имени...");
    familySearchBox->setMaximumHeight(32);
    familyLayout->addWidget(familySearchBox);

    // Список семьи
    familyList = new QListWidget();
    familyList->setMinimumHeight(250);
    // Allow double-click to edit a family member
    familyList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(familyList, &QListWidget::itemDoubleClicked, this, &ProfileWidget::onEditFamilyMember);

    familyLayout->addWidget(familyList);

    // Кнопки действия
    QHBoxLayout *familyActions = new QHBoxLayout();
    familyActions->setSpacing(8);
    
    removeFamilyButton = new QPushButton("Удалить из семьи");
    removeFamilyButton->setIcon(QIcon(":/images/icon-trash.svg"));
    removeFamilyButton->setIconSize(QSize(16, 16));
    removeFamilyButton->setMinimumHeight(36);
    removeFamilyButton->setProperty("class", "danger-button");

    familyActions->addWidget(removeFamilyButton);
    familyActions->addStretch();

    familyLayout->addLayout(familyActions);

    addFamilyStatus = new QLabel();
    addFamilyStatus->setProperty("class", "status-success");
    familyLayout->addWidget(addFamilyStatus);

    scrollable->addContent(familyCard);

    // ===== Коды приглашения =====
    QWidget *codesCard = new QWidget();
    codesCard->setProperty("class", "modern-card");
    QVBoxLayout *codesLayout = new QVBoxLayout(codesCard);
    codesLayout->setContentsMargins(20, 16, 20, 16);
    codesLayout->setSpacing(12);

    QLabel *codesTitle = new QLabel("Пригласить в семью");
    codesTitle->setProperty("class", "section-title");
    codesLayout->addWidget(codesTitle);

    // Генерация кода
    generateCodeButton = new QPushButton("Сгенерировать код");
    generateCodeButton->setIcon(QIcon(":/images/icon-lock.svg"));
    generateCodeButton->setIconSize(QSize(16, 16));
    generateCodeButton->setMinimumHeight(36);
    invitationCodeDisplay = new QLineEdit();
    invitationCodeDisplay->setText("Нет активного кода");
    invitationCodeDisplay->setReadOnly(true);
    invitationCodeDisplay->setAlignment(Qt::AlignCenter);
    invitationCodeDisplay->setProperty("class", "invitation-code");

    codesLayout->addWidget(new QLabel("Выпущенный код:"));
    codesLayout->addWidget(invitationCodeDisplay);
    codesLayout->addWidget(generateCodeButton);

    codesLayout->addSpacing(16);

    // Использование кода
    QLabel *useCodeLabel = new QLabel("Присоединиться к семье:");
    useCodeLabel->setProperty("class", "use-code-label");
    codesLayout->addWidget(useCodeLabel);

    useCodeButton = new QPushButton("Открыть диалог");
    useCodeButton->setIcon(QIcon(":/images/icon-unlock.svg"));
    useCodeButton->setIconSize(QSize(16, 16));
    useCodeButton->setMinimumHeight(36);

    codesLayout->addWidget(useCodeButton);

    codeStatusLabel = new QLabel();
    codeStatusLabel->setProperty("class", "status-label");
    codesLayout->addWidget(codeStatusLabel);

    scrollable->addContent(codesCard);
    scrollable->addStretch();

    profileLayout->addWidget(scrollable);
    tabs->addTab(profileTab, "Профиль");
}

void ProfileWidget::buildSettingsTab() {
    QWidget *settingsTab = new QWidget();
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsTab);
    settingsLayout->setContentsMargins(0, 0, 0, 0);
    settingsLayout->setSpacing(0);

    ScrollableTab *scrollable = new ScrollableTab();

    // Карточка редактирования
    QWidget *settingsCard = new QWidget();
    settingsCard->setProperty("class", "modern-card");
    QVBoxLayout *cardLayout = new QVBoxLayout(settingsCard);
    cardLayout->setContentsMargins(20, 16, 20, 16);
    cardLayout->setSpacing(16);

    QLabel *title = new QLabel("Редактировать профиль");
    title->setProperty("class", "info-title");
    cardLayout->addWidget(title);

    QFormLayout *form = new QFormLayout();
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(12);

    firstNameEdit = new QLineEdit();
    firstNameEdit->setPlaceholderText("Имя");
    lastNameEdit = new QLineEdit();
    lastNameEdit->setPlaceholderText("Фамилия");
    middleNameEdit = new QLineEdit();
    middleNameEdit->setPlaceholderText("Отчество");
    birthEdit = new QDateEdit();
    birthEdit->setCalendarPopup(true);
    birthEdit->setDisplayFormat("yyyy-MM-dd");
    emailEdit = new QLineEdit();
    emailEdit->setPlaceholderText("email@example.com");
    phoneEdit = new QLineEdit();
    phoneEdit->setPlaceholderText("+7...");
    snilsEdit = new QLineEdit();
    snilsEdit->setPlaceholderText("СНИЛС");
    omsEdit = new QLineEdit();
    omsEdit->setPlaceholderText("Полис ОМС");

    form->addRow("Имя", firstNameEdit);
    form->addRow("Фамилия", lastNameEdit);
    form->addRow("Отчество", middleNameEdit);
    form->addRow("Дата рождения", birthEdit);
    form->addRow("Email", emailEdit);
    form->addRow("Телефон", phoneEdit);
    form->addRow("СНИЛС", snilsEdit);
    form->addRow("Полис ОМС", omsEdit);

    cardLayout->addLayout(form);

    saveButton = new QPushButton("Сохранить");
    saveButton->setIcon(QIcon(":/images/icon-save.svg"));
    saveButton->setIconSize(QSize(18, 18));
    saveButton->setMinimumHeight(40);
    deleteButton = new QPushButton("Удалить аккаунт");
    deleteButton->setIcon(QIcon(":/images/icon-trash.svg"));
    deleteButton->setIconSize(QSize(18, 18));
    deleteButton->setMinimumHeight(40);
    deleteButton->setProperty("class", "danger-button");
    
    refreshDataButton = new QPushButton("Обновить данные");
    refreshDataButton->setIcon(QIcon(":/images/icon-refresh.svg"));
    refreshDataButton->setIconSize(QSize(18, 18));
    refreshDataButton->setMinimumHeight(40);

    saveStatusLabel = new QLabel();
    saveStatusLabel->setProperty("class", "status-success");

    QHBoxLayout *actionsLayout = new QHBoxLayout();
    actionsLayout->addWidget(saveButton);
    actionsLayout->addStretch();
    actionsLayout->addWidget(deleteButton);
    
    QHBoxLayout *topActionsLayout = new QHBoxLayout();
    topActionsLayout->addWidget(refreshDataButton);
    topActionsLayout->addStretch();

    cardLayout->addLayout(topActionsLayout);
    cardLayout->addLayout(actionsLayout);
    cardLayout->addWidget(saveStatusLabel);

    scrollable->addContent(settingsCard);
    scrollable->addStretch();

    settingsLayout->addWidget(scrollable);
    tabs->addTab(settingsTab, "Настройки");
}

void ProfileWidget::connectSignals() {
    connect(removeFamilyButton, &QPushButton::clicked, this, &ProfileWidget::onRemoveFamilyMember);
    connect(generateCodeButton, &QPushButton::clicked, this, &ProfileWidget::onGenerateInvitationCode);
    connect(useCodeButton, &QPushButton::clicked, this, &ProfileWidget::onUseInvitationCode);
    connect(saveButton, &QPushButton::clicked, this, &ProfileWidget::onSaveProfile);
    connect(deleteButton, &QPushButton::clicked, this, &ProfileWidget::onDeleteAccount);
    connect(refreshDataButton, &QPushButton::clicked, this, &ProfileWidget::onRefreshData);
    
    // Поиск в семье с real-time фильтром
    if (familySearchBox) {
        connect(familySearchBox, &QLineEdit::textChanged, this, [this](const QString &text) {
            for (int i = 0; i < familyList->count(); ++i) {
                QListWidgetItem *item = familyList->item(i);
                bool matches = item->text().contains(text, Qt::CaseInsensitive);
                item->setHidden(!matches && !text.isEmpty());
            }
        });
    }
}

void ProfileWidget::setUser(const LoginUser &user) {
    currentUser = user;
    loadProfile();
    loadFamily();
    
    // Загружаем активный код приглашения если есть
    if (currentUser.type == LoginUser::PATIENT) {
        QList<InvitationCode> codes = dataManager.getInvitationCodes(currentUser.id);
        for (const InvitationCode &code : codes) {
            if (!code.used) {
                invitationCodeDisplay->setText(code.code);
                break;
            }
        }
    }
}

void ProfileWidget::openSettingsTab() {
    tabs->setCurrentIndex(1);
}

void ProfileWidget::loadProfile() {
    nameValue->setText("—");
    emailValue->setText("—");
    phoneValue->setText("—");
    birthValue->setText("—");

    switch (currentUser.type) {
        case LoginUser::PATIENT: {
            Patient p = dataManager.getPatientById(currentUser.id);
            nameValue->setText(p.fullName());
            emailValue->setText(p.email);
            phoneValue->setText(p.phone_number);
            birthValue->setText(p.bdate);
            
            firstNameEdit->setText(p.fname);
            lastNameEdit->setText(p.lname);
            middleNameEdit->setText(p.tname);
            birthEdit->setDate(QDate::fromString(p.bdate, "yyyy-MM-dd"));
            emailEdit->setText(p.email);
            phoneEdit->setText(p.phone_number);
            snilsEdit->setText(p.snils);
            omsEdit->setText(p.oms);
            break;
        }
        case LoginUser::DOCTOR: {
            Doctor d = dataManager.getDoctorById(currentUser.id);
            nameValue->setText(d.fullName());
            emailValue->setText(d.email);
            phoneValue->setText(d.phone_number);
            birthValue->setText(d.bdate.toString("yyyy-MM-dd"));
            
            firstNameEdit->setText(d.fname);
            lastNameEdit->setText(d.lname);
            middleNameEdit->setText(d.tname);
            birthEdit->setDate(d.bdate);
            emailEdit->setText(d.email);
            phoneEdit->setText(d.phone_number);
            snilsEdit->clear();
            omsEdit->clear();
            break;
        }
        case LoginUser::MANAGER: {
            Manager m = dataManager.getManagerById(currentUser.id);
            nameValue->setText(m.fullName());
            emailValue->setText(m.email);
            phoneValue->setText("—");
            
            firstNameEdit->setText(m.fname);
            lastNameEdit->setText(m.lname);
            middleNameEdit->clear();
            birthEdit->setDate(QDate::currentDate());
            emailEdit->setText(m.email);
            phoneEdit->clear();
            snilsEdit->clear();
            omsEdit->clear();
            break;
        }
        default:
            break;
    }
}

void ProfileWidget::loadFamily() {
    familyList->clear();
    addFamilyStatus->clear();

    if (currentUser.type != LoginUser::PATIENT) {
        familyList->addItem("Доступно только для пациентов");
        if (familyCountBadge) familyCountBadge->setValue(0);
        return;
    }
    
    // Получаем всех членов семьи (и как родитель, и как ребенок)
    QList<PatientGroup> allMembers = dataManager.getPatientFamilyMembers(currentUser.id);
    
    // Также ищем, есть ли текущий пользователь в роли ребенка (приглашенный)
    QList<PatientGroup> parentGroups = dataManager.getPatientParents(currentUser.id);
    for (const PatientGroup& pg : parentGroups) {
        allMembers.append(pg);
    }
    
    // Подслеживаем уникальные ID пациентов чтобы не добавлять дубликаты
    QSet<int> addedIds;

    for (const PatientGroup &pg : allMembers) {
        int patientId = -1;
        
        // Если текущий пользователь - родитель, показываем детей
        if (pg.id_parent == currentUser.id) {
            patientId = pg.id_child;
        }
        // Если текущий пользователь - ребенок, показываем родителя
        else if (pg.id_child == currentUser.id) {
            patientId = pg.id_parent;
        }
        
        // Проверяем, не добавили ли уже этого пациента
        if (patientId > 0 && !addedIds.contains(patientId)) {
            addedIds.insert(patientId);
        }
    }

    if (addedIds.isEmpty()) {
        familyList->addItem("Нет добавленных членов семьи");
        if (familyCountBadge) familyCountBadge->setValue(0);
        return;
    }

    // Устанавливаем корректный счетчик - количество уникальных членов семьи
    if (familyCountBadge) familyCountBadge->setValue(addedIds.size());

    // Снова проходим по членам семьи для добавления в список
    QSet<int> displayedIds;
    for (const PatientGroup &pg : allMembers) {
        int patientId = -1;
        
        // Если текущий пользователь - родитель, показываем детей
        if (pg.id_parent == currentUser.id) {
            patientId = pg.id_child;
        }
        // Если текущий пользователь - ребенок, показываем родителя
        else if (pg.id_child == currentUser.id) {
            patientId = pg.id_parent;
        }
        
        // Проверяем, не добавили ли уже этого пациента
        if (patientId > 0 && !displayedIds.contains(patientId)) {
            displayedIds.insert(patientId);
            
            Patient p = dataManager.getPatientById(patientId);
            QString name = p.fullName();
            if (name.trimmed().isEmpty()) {
                name = QString("Пациент #%1").arg(patientId);
            }
            
            QListWidgetItem *item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, patientId);
            familyList->addItem(item);
        }
    }
}

void ProfileWidget::onRemoveFamilyMember() {
    if (currentUser.type != LoginUser::PATIENT) {
        addFamilyStatus->setText("Удаление доступно только для пациентов");
        return;
    }

    // Проверяем, выбран ли член семьи
    QListWidgetItem *currentItem = familyList->currentItem();
    if (!currentItem) {
        addFamilyStatus->setText("Выберите члена семьи для удаления");
        return;
    }

    int selectedPatientId = currentItem->data(Qt::UserRole).toInt();
    
    // Подтверждение удаления
    auto reply = QMessageBox::question(this, "Удаление из семьи",
                                       QString("Вы уверены, что хотите удалить этого члена семьи?"));
    if (reply != QMessageBox::Yes) {
        return;
    }

    // Находим и удаляем связь между пациентами
    QList<PatientGroup> familyGroups = dataManager.getPatientFamilyMembers(currentUser.id);
    for (const PatientGroup &pg : familyGroups) {
        if (pg.id_child == selectedPatientId) {
            dataManager.removeFamilyMember(pg.id_patient_group);
            addFamilyStatus->setText("Член семьи удален");
            loadFamily();
            return;
        }
    }

    addFamilyStatus->setText("Не удалось удалить члена семьи");
}

void ProfileWidget::onEditFamilyMember(QListWidgetItem *item) {
    if (currentUser.type != LoginUser::PATIENT) {
        addFamilyStatus->setText("Редактирование доступно только для пациентов");
        return;
    }

    if (!item) return;
    int patientId = item->data(Qt::UserRole).toInt();
    if (patientId <= 0) return;

    Patient p = dataManager.getPatientById(patientId);
    // Open CreatePatientDialog in edit mode
    CreatePatientDialog dlg(this, &p);
    connect(&dlg, &CreatePatientDialog::patientCreated, this, [&](const Patient &updated){
        Q_UNUSED(updated);
        loadFamily();
        addFamilyStatus->setText("Данные члена семьи обновлены");
    });
    dlg.exec();
}

void ProfileWidget::onRefreshData() {
    loadProfile();
    loadFamily();
    
    if (currentUser.type == LoginUser::PATIENT) {
        // Обновляем активный код приглашения
        QList<InvitationCode> codes = dataManager.getInvitationCodes(currentUser.id);
        invitationCodeDisplay->setText("Нет активного кода");
        for (const InvitationCode &code : codes) {
            if (!code.used) {
                invitationCodeDisplay->setText(code.code);
                break;
            }
        }
    }
}

void ProfileWidget::onSaveProfile() {
    saveStatusLabel->clear();
    switch (currentUser.type) {
        case LoginUser::PATIENT: {
            Patient p = dataManager.getPatientById(currentUser.id);
            if (p.id_patient <= 0) break;
            p.fname = firstNameEdit->text().trimmed();
            p.lname = lastNameEdit->text().trimmed();
            p.tname = middleNameEdit->text().trimmed();
            p.bdate = birthEdit->date().toString("yyyy-MM-dd");
            p.email = emailEdit->text().trimmed();
            p.phone_number = phoneEdit->text().trimmed();
            p.snils = snilsEdit->text().trimmed();
            p.oms = omsEdit->text().trimmed();
            dataManager.updatePatient(p);
            saveStatusLabel->setText("Данные обновлены");
            loadProfile();
            break;
        }
        case LoginUser::DOCTOR: {
            Doctor d = dataManager.getDoctorById(currentUser.id);
            if (d.id_doctor <= 0) break;
            d.fname = firstNameEdit->text().trimmed();
            d.lname = lastNameEdit->text().trimmed();
            d.tname = middleNameEdit->text().trimmed();
            d.bdate = birthEdit->date();
            d.email = emailEdit->text().trimmed();
            d.phone_number = phoneEdit->text().trimmed();
            dataManager.updateDoctor(d);
            saveStatusLabel->setText("Данные обновлены");
            loadProfile();
            break;
        }
        case LoginUser::MANAGER: {
            Manager m = dataManager.getManagerById(currentUser.id);
            if (m.id <= 0) break;
            m.fname = firstNameEdit->text().trimmed();
            m.lname = lastNameEdit->text().trimmed();
            m.email = emailEdit->text().trimmed();
            dataManager.updateManager(m);
            saveStatusLabel->setText("Данные обновлены");
            loadProfile();
            break;
        }
        default:
            break;
    }
}

void ProfileWidget::onDeleteAccount() {
    auto reply = QMessageBox::question(this, "Удаление аккаунта",
                                       "Вы уверены, что хотите удалить аккаунт? Это действие необратимо.");
    if (reply != QMessageBox::Yes) {
        return;
    }

    switch (currentUser.type) {
        case LoginUser::PATIENT:
            dataManager.deletePatient(currentUser.id);
            break;
        case LoginUser::DOCTOR:
            dataManager.deleteDoctor(currentUser.id);
            break;
        case LoginUser::MANAGER:
            dataManager.deleteManager(currentUser.id);
            break;
        default:
            break;
    }

    emit requestAccountDeletion();
}

void ProfileWidget::onGenerateInvitationCode() {
    QString code = dataManager.generateInvitationCode(currentUser.id);
    invitationCodeDisplay->setText(code);
    addFamilyStatus->setText(QString("Код приглашения создан: %1").arg(code));
}

void ProfileWidget::onUseInvitationCode() {
    // Открываем модальный диалог для присоединения по коду
    AddByCodeDialog dlg(currentUser.id, this);
    
    connect(&dlg, &AddByCodeDialog::patientAdded, this, [this](int patientId, const QString &patientName) {
        Q_UNUSED(patientId);
        Q_UNUSED(patientName);
        codeStatusLabel->setText("Вы успешно добавлены в семью!");
        codeStatusLabel->setProperty("class", "status-label success");
        loadFamily();
        onRefreshData();
    });
    
    dlg.exec();
}