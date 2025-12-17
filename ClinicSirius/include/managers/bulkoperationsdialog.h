#ifndef BULKOPERATIONSDIALOG_H
#define BULKOPERATIONSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCompleter>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include "common/datamanager.h"
#include "common/models.h"

class BulkOperationsDialog : public QDialog {
    Q_OBJECT
public:
    explicit BulkOperationsDialog(QWidget* parent = nullptr);

private slots:
    void onApply();

private:
    void buildUI();
    void loadDoctors();
    void loadRooms();
    
    DataManager m_dataManager;
    QLineEdit* m_doctorEdit;
    QCompleter* m_doctorCompleter;
    QComboBox* m_roomCombo;
    QCompleter* m_roomCompleter;
    QList<Room> m_rooms;
    QDateEdit* m_dateEdit;
    QTimeEdit* m_startTimeEdit;
    QTimeEdit* m_endTimeEdit;
    QCheckBox* m_lunchCheckBox;
    QTimeEdit* m_lunchFromEdit;
    QTimeEdit* m_lunchToEdit;
    QSpinBox* m_intervalSpin; // minutes
    QPushButton* m_applyBtn;
    QPushButton* m_cancelBtn;
};

#endif // BULKOPERATIONSDIALOG_H
