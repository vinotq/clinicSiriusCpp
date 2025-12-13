#ifndef ADDBYCODEDIALOG_H
#define ADDBYCODEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "models.h"
#include "datamanager.h"

/**
 * @brief AddByCodeDialog - диалог для присоединения к семье по коду приглашения
 * 
 * Позволяет присоединиться к существующей семье используя:
 * - Код приглашения (6 символов)
 */
class AddByCodeDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddByCodeDialog(int patientId, QWidget *parent = nullptr);
    ~AddByCodeDialog() = default;

    /**
     * @brief Получить ID главы семьи
     */
    int getHeadPatientId() const { return headPatientId; }

signals:
    void patientAdded(int patientId, const QString &patientName);

private slots:
    void onAddClicked();

private:
    void setupUI();

    DataManager dataManager;
    int patientId;              // ID пациента, который присоединяется
    int headPatientId;          // ID главы семьи

    QLineEdit *codeInput;       // Для кода приглашения
    QLabel *errorLabel;
    QPushButton *addButton;
};

#endif // ADDBYCODEDIALOG_H

