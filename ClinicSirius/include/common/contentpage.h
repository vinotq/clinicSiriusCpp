#ifndef CONTENTPAGE_H
#define CONTENTPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class ContentPage : public QWidget {
    Q_OBJECT

public:
    explicit ContentPage(QWidget *parent = nullptr);
    ~ContentPage();

    void setTitle(const QString &title);
    void setContent(QWidget *widget);

private:
    void setupUI();
    void applyStyles();

    QLabel *titleLabel;
    QVBoxLayout *contentLayout;
};

#endif // CONTENTPAGE_H
