#ifndef STATSBADGE_H
#define STATSBADGE_H

#include <QWidget>
#include <QLabel>

class StatsBadge : public QWidget {
    Q_OBJECT

public:
    enum BadgeType {
        Primary,
        Success,
        Warning,
        Danger,
        Info
    };

    explicit StatsBadge(QWidget *parent = nullptr);
    ~StatsBadge() = default;

    void setValue(int value);

    void setLabel(const QString &label);

    void setIcon(const QString &iconPath);

    void setBadgeType(BadgeType type);

    void setDescription(const QString &description);

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

#endif
