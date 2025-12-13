#ifndef STATSBADGE_H
#define STATSBADGE_H

#include <QWidget>
#include <QLabel>

/**
 * @brief StatsBadge - компактная карточка со статистикой
 * 
 * Используется для отображения:
 * - Количеств (приёмы, пациенты)
 * - Статусов (онлайн, занят)
 * - Кратких метрик
 */
class StatsBadge : public QWidget {
    Q_OBJECT

public:
    enum BadgeType {
        Primary,      // Синий - основной статус
        Success,      // Зелёный - успешно
        Warning,      // Жёлтый - внимание
        Danger,       // Красный - критично
        Info          // Голубой - информация
    };

    explicit StatsBadge(QWidget *parent = nullptr);
    ~StatsBadge() = default;

    /**
     * @brief Установить значение метрики
     * @param value Числовое значение
     */
    void setValue(int value);

    /**
     * @brief Установить подпись
     * @param label Текст подписи
     */
    void setLabel(const QString &label);

    /**
     * @brief Установить иконку
     * @param iconPath Путь к иконке
     */
    void setIcon(const QString &iconPath);

    /**
     * @brief Установить тип оформления
     * @param type Тип бейджа
     */
    void setBadgeType(BadgeType type);

    /**
     * @brief Установить описание при наведении
     * @param description Текст подсказки
     */
    void setDescription(const QString &description);

    /**
     * @brief Получить текущее значение
     */
    int getValue() const;

private:
    void setupUI();
    void updateStyle();
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

    QLabel *valueLabel;
    QLabel *labelLabel;
    QLabel *iconLabel;
    BadgeType badgeType;
    int currentValue;
    QString description;
};

#endif // STATSBADGE_H
