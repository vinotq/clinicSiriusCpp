#ifndef BULKOPERATIONSDIALOG_H
#define BULKOPERATIONSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QPushButton>
#include "common/datamanager.h"

class BulkOperationsDialog : public QDialog {
    Q_OBJECT
public:
    explicit BulkOperationsDialog(QWidget* parent = nullptr);

private slots:
    void onApply();

private:
    void buildUI();
    DataManager m_dataManager;
    QComboBox* m_doctorCombo;
    QDateEdit* m_dateEdit;
    QTimeEdit* m_startTimeEdit;
    QTimeEdit* m_endTimeEdit;
    QTimeEdit* m_lunchFromEdit;
    QTimeEdit* m_lunchToEdit;
    QSpinBox* m_intervalSpin; // minutes
    QPushButton* m_applyBtn;
    QPushButton* m_cancelBtn;
};

#endif // BULKOPERATIONSDIALOG_H
