#ifndef DETAILMENU_H
#define DETAILMENU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPropertyAnimation>

/**
 * @brief DetailMenu - боковое меню для дополнительных данных
 * 
 * Компактное меню в боку экрана, которое:
 * - Плавно выдвигается/скрывается
 * - Содержит редко используемые поля
 * - Имеет иконку-кнопку для открытия
 */
class DetailMenu : public QWidget {
    Q_OBJECT

public:
    explicit DetailMenu(QWidget *parent = nullptr);
    ~DetailMenu() = default;

    /**
     * @brief Добавить элемент в меню
     * @param label Название поля
     * @param value Значение поля
     * @param icon Путь к иконке (опционально)
     */
    void addItem(const QString &label, const QString &value, const QString &icon = "");

    /**
     * @brief Добавить элемент меню с виджетом
     * @param label Название
     * @param widget Кастомный виджет
     */
    void addCustomItem(const QString &label, QWidget *widget);

    /**
     * @brief Очистить меню
     */
    void clearItems();

    /**
     * @brief Показать меню с анимацией
     */
    void showMenu();

    /**
     * @brief Скрыть меню с анимацией
     */
    void hideMenu();

    /**
     * @brief Переключить видимость меню
     */
    void toggleMenu();

    /**
     * @brief Проверить, открыто ли меню
     */
    bool isVisible() const;

    /**
     * @brief Установить ширину меню
     */
    void setMenuWidth(int width);

signals:
    void menuOpenedChanged(bool opened);

private:
    void setupUI();
    void setupAnimation();
    void connectSignals();

    QWidget *menuContent;
    QVBoxLayout *menuLayout;
    QPushButton *toggleButton;
    QPropertyAnimation *slideAnimation;
    bool isOpen;
    int menuWidth;
};

#endif // DETAILMENU_H
