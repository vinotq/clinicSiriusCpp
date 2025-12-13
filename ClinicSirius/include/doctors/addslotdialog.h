#ifndef ADDSLOTDIALOG_H
#define ADDSLOTDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QComboBox>
#include "models.h"
#include "datamanager.h"

class AddSlotDialog : public QDialog {
    Q_OBJECT
public:
    // Старый конструктор
    explicit AddSlotDialog(int doctorId, QWidget *parent = nullptr);
    // Новый конструктор с параметрами по умолчанию
    AddSlotDialog(int doctorId, QTime defaultStartTime, int defaultDurationMin, QWidget *parent = nullptr);

signals:
    void slotAdded();

private slots:
    void onAddSlot();

private:
    void buildUI();
    void loadRooms();

    int doctorId;
    DataManager dataManager;

    QDateEdit *dateEdit;
    QTimeEdit *startTimeEdit;
    QSpinBox *durationSpinBox;
    QComboBox *roomCombo;
    QPushButton *addButton;
    QPushButton *cancelButton;

    QTime _defaultStartTime = QTime(10, 0); // по умолчанию 10:00, но можно менять
    int _defaultDurationMin = 60;           // по умолчанию 60 минут, но можно менять
};

#endif // ADDSLOTDIALOG_H
