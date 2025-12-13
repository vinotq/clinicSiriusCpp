#ifndef SCROLLABLETAB_H
#define SCROLLABLETAB_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

/**
 * @brief ScrollableTab - базовый контейнер с плавной прокруткой
 * 
 * Предоставляет удобный контейнер с поддержкой:
 * - Плавной прокрутки
 * - Автоматического разворачивания содержимого
 * - Кастомных стилей для современного вида
 */
class ScrollableTab : public QWidget {
    Q_OBJECT

public:
    explicit ScrollableTab(QWidget *parent = nullptr);
    ~ScrollableTab() = default;

    /**
     * @brief Добавить виджет в прокручиваемое содержимое
     * @param widget Виджет для добавления
     */
    void addContent(QWidget *widget);

    /**
     * @brief Добавить виджет с растяжением по ширине
     * @param widget Виджет для добавления
     * @param stretch Коэффициент растяжения (по умолчанию 0)
     */
    void addStretchedContent(QWidget *widget, int stretch = 0);

    /**
     * @brief Добавить растягиваемое пространство
     * @param stretch Коэффициент растяжения
     */
    void addStretch(int stretch = 1);

    /**
     * @brief Получить основной layout прокручиваемого содержимого
     * @return Указатель на QVBoxLayout
     */
    QVBoxLayout *contentLayout() const;

    /**
     * @brief Очистить всё содержимое
     */
    void clearContent();

    /**
     * @brief Установить отступы содержимого
     * @param left Левый отступ
     * @param top Верхний отступ
     * @param right Правый отступ
     * @param bottom Нижний отступ
     */
    void setContentMargins(int left, int top, int right, int bottom);

    /**
     * @brief Установить интервал между элементами
     * @param spacing Размер интервала в пиксель
     */
    void setContentSpacing(int spacing);

    /**
     * @brief Прокрутить к верхней части
     */
    void scrollToTop();

    /**
     * @brief Прокрутить к нижней части
     */
    void scrollToBottom();

private:
    void setupUI();
    void setupStyles();

    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QVBoxLayout *mainLayout;
    QVBoxLayout *contentLayout_;
};

#endif // SCROLLABLETAB_H
