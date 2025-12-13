#ifndef INFOCARD_H
#define INFOCARD_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>

class InfoCard : public QWidget {
    Q_OBJECT

public:
    explicit InfoCard(const QString &title, const QString &description,
                     const QString &iconPath = "", QWidget *parent = nullptr);
    ~InfoCard();

    void setTitle(const QString &title);
    void setDescription(const QString &description);
    void setIcon(const QString &iconPath);

private:
    void setupUI();
    void applyStyles();

    QLabel *iconLabel;
    QLabel *titleLabel;
    QLabel *descriptionLabel;
};

#endif // INFOCARD_H
