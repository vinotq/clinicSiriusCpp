#ifndef SCROLLABLETAB_H
#define SCROLLABLETAB_H

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class ScrollableTab : public QWidget {
    Q_OBJECT

public:
    explicit ScrollableTab(QWidget *parent = nullptr);
    ~ScrollableTab() = default;

    void addContent(QWidget *widget);

    void addStretchedContent(QWidget *widget, int stretch = 0);

    void addStretch(int stretch = 1);

    QVBoxLayout *contentLayout() const;

    void clearContent();

    void setContentMargins(int left, int top, int right, int bottom);

    void setContentSpacing(int spacing);

    void scrollToTop();

    void scrollToBottom();

private:
    void setupUI();
    void setupStyles();

    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QVBoxLayout *mainLayout;
    QVBoxLayout *contentLayout_;
};

#endif
