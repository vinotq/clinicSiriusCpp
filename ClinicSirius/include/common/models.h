#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRandomGenerator>

// Функции для работы с паролями с солью
inline QString generateSalt() {
    QRandomGenerator gen = QRandomGenerator::securelySeeded();
    QByteArray salt;
    for (int i = 0; i < 16; ++i) {
        salt.append(static_cast<char>(gen.bounded(0, 256)));
    }
    return QString::fromUtf8(salt.toHex());
}

inline QString hashPassword(const QString& password, const QString& salt = QString()) {
    QString salted = password + salt;
    QString hash = QString::fromUtf8(QCryptographicHash::hash(salted.toUtf8(), QCryptographicHash::Sha256).toHex());
    if (salt.isEmpty()) {
        // Legacy: if no salt provided, generate new one
        QString newSalt = generateSalt();
        return hash + ":" + newSalt;
    }
    return hash + ":" + salt;
}

inline bool verifyPassword(const QString& password, const QString& stored) {
    QStringList parts = stored.split(":");
    if (parts.size() != 2) {
        // Legacy: no salt format, compare directly
        return QString::fromUtf8(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex()) == stored;
    }
    QString salt = parts[1];
    return hashPassword(password, salt) == stored;
}

// Структура для хранения информации о пользователе
struct LoginUser {
    enum UserType { PATIENT = 0, DOCTOR = 1, MANAGER = 2, ADMIN = 3 };

    UserType type;
    int id;
    QString name;

    LoginUser() : type(PATIENT), id(-1) {}
    LoginUser(UserType t, int uid, const QString& uname)
        : type(t), id(uid), name(uname) {}
};

struct Patient {
    int id_patient;
    QString fname;
    QString lname;
    QString tname;
    QString bdate;
    QString phone_number;
    QString email;
    QString snils;
    QString oms;
    QString password;  // Хеш пароля

    QString fullName() const {
        // Display as: Фамилия Имя Отчество
        return lname + " " + fname + (tname.isEmpty() ? "" : " " + tname);
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_patient"] = id_patient;
        obj["fname"] = fname;
        obj["lname"] = lname;
        obj["tname"] = tname;
        obj["bdate"] = bdate;
        obj["phone_number"] = phone_number;
        obj["email"] = email;
        obj["snils"] = snils;
        obj["oms"] = oms;
        obj["password"] = password;
        return obj;
    }

    static Patient fromJson(const QJsonObject& obj) {
        Patient p;
        p.id_patient = obj["id_patient"].toInt();
        p.fname = obj["fname"].toString();
        p.lname = obj["lname"].toString();
        p.tname = obj["tname"].toString();
        p.bdate = obj["bdate"].toString();
        p.phone_number = obj["phone_number"].toString();
        p.email = obj["email"].toString();
        p.snils = obj["snils"].toString();
        p.oms = obj["oms"].toString();
        p.password = obj["password"].toString();
        return p;
    }
};

struct Doctor {
    int id_doctor;
    QString fname;
    QString lname;
    QString tname;
    QDate bdate;
    QString phone_number;
    QString email;
    int id_spec;
    QString password;  // Хеш пароля

    QString fullName() const {
        // Display as: Фамилия Имя Отчество
        return lname + " " + fname + (tname.isEmpty() ? "" : " " + tname);
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_doctor"] = id_doctor;
        obj["fname"] = fname;
        obj["lname"] = lname;
        obj["tname"] = tname;
        obj["bdate"] = bdate.toString("yyyy-MM-dd");
        obj["phone_number"] = phone_number;
        obj["email"] = email;
        obj["id_spec"] = id_spec;
        obj["password"] = password;
        return obj;
    }

    static Doctor fromJson(const QJsonObject& obj) {
        Doctor d;
        d.id_doctor = obj["id_doctor"].toInt();
        d.fname = obj["fname"].toString();
        d.lname = obj["lname"].toString();
        d.tname = obj["tname"].toString();
        d.bdate = QDate::fromString(obj["bdate"].toString(), "yyyy-MM-dd");
        d.phone_number = obj["phone_number"].toString();
        d.email = obj["email"].toString();
        d.id_spec = obj["id_spec"].toInt();
        d.password = obj["password"].toString();
        return d;
    }
};

struct Specialization {
    int id_spec;
    QString name;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_spec"] = id_spec;
        obj["name"] = name;
        return obj;
    }

    static Specialization fromJson(const QJsonObject& obj) {
        Specialization s;
        s.id_spec = obj["id_spec"].toInt();
        s.name = obj["name"].toString();
        return s;
    }
};

struct Room {
    int id_room;
    QString room_number;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_room"] = id_room;
        obj["room_number"] = room_number;
        return obj;
    }

    static Room fromJson(const QJsonObject& obj) {
        Room r;
        r.id_room = obj["id_room"].toInt();
        r.room_number = obj["room_number"].toString();
        return r;
    }
};

struct AppointmentSchedule {
    int id_ap_sch;
    int id_doctor;
    int id_room;
    QDateTime time_from;
    QDateTime time_to;
    QString status = "free";  // "free" или "booked"

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_ap_sch"] = id_ap_sch;
        obj["id_doctor"] = id_doctor;
        obj["id_room"] = id_room;
        obj["time_from"] = time_from.toString("yyyy-MM-ddTHH:mm:ss");
        obj["time_to"] = time_to.toString("yyyy-MM-ddTHH:mm:ss");
        obj["status"] = status;
        return obj;
    }

    static AppointmentSchedule fromJson(const QJsonObject& obj) {
        AppointmentSchedule a;
        a.id_ap_sch = obj["id_ap_sch"].toInt();
        a.id_doctor = obj["id_doctor"].toInt();
        a.id_room = obj["id_room"].toInt();
        a.time_from = QDateTime::fromString(obj["time_from"].toString(), "yyyy-MM-ddTHH:mm:ss");
        a.time_to = QDateTime::fromString(obj["time_to"].toString(), "yyyy-MM-ddTHH:mm:ss");
        a.status = obj["status"].toString("free");
        return a;
    }
};

struct Appointment {
    int id_ap;
    int id_patient;
    int id_doctor;
    int id_ap_sch = -1;  // Link to appointment schedule
    QDateTime date;
    bool completed = false;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_ap"] = id_ap;
        obj["id_patient"] = id_patient;
        obj["id_doctor"] = id_doctor;
        obj["id_ap_sch"] = id_ap_sch;
        obj["date"] = date.toString("yyyy-MM-ddTHH:mm:ss");
        obj["completed"] = completed;
        return obj;
    }

    static Appointment fromJson(const QJsonObject& obj) {
        Appointment a;
        a.id_ap = obj["id_ap"].toInt();
        a.id_patient = obj["id_patient"].toInt();
        a.id_doctor = obj["id_doctor"].toInt();
        a.id_ap_sch = obj["id_ap_sch"].toInt(-1);
        a.date = QDateTime::fromString(obj["date"].toString(), "yyyy-MM-ddTHH:mm:ss");
        a.completed = obj["completed"].toBool(false);
        return a;
    }
};

struct Diagnosis {
    int id_diagnosis;
    QString name;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_diagnosis"] = id_diagnosis;
        obj["name"] = name;
        return obj;
    }

    static Diagnosis fromJson(const QJsonObject& obj) {
        Diagnosis d;
        d.id_diagnosis = obj["id_diagnosis"].toInt();
        d.name = obj["name"].toString();
        return d;
    }
};

struct Recipe {
    int id;
    int id_ap;
    int id_diagnosis;
    QString complaints;
    QString recommendations;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["id_ap"] = id_ap;
        obj["id_diagnosis"] = id_diagnosis;
        obj["complaints"] = complaints;
        obj["recommendations"] = recommendations;
        return obj;
    }

    static Recipe fromJson(const QJsonObject& obj) {
        Recipe r;
        r.id = obj["id"].toInt();
        r.id_ap = obj["id_ap"].toInt();
        r.id_diagnosis = obj["id_diagnosis"].toInt();
        r.complaints = obj["complaints"].toString();
        r.recommendations = obj["recommendations"].toString();
        return r;
    }
};

struct PatientGroup {
    int id_patient_group;
    int id_parent;
    int id_child;
    int family_head;          // ID главы семьи (того, кто создал семью)

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id_patient_group"] = id_patient_group;
        obj["id_parent"] = id_parent;
        obj["id_child"] = id_child;
        obj["family_head"] = family_head;
        return obj;
    }

    static PatientGroup fromJson(const QJsonObject& obj) {
        PatientGroup pg;
        pg.id_patient_group = obj["id_patient_group"].toInt();
        pg.id_parent = obj["id_parent"].toInt();
        pg.id_child = obj["id_child"].toInt();
        pg.family_head = obj["family_head"].toInt();
        return pg;
    }
};

struct Manager {
    int id = -1;
    QString fname;
    QString lname;
    QString email;
    QString password;

    QString fullName() const {
        // Display as: Фамилия Имя
        return lname + " " + fname;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["fname"] = fname;
        obj["lname"] = lname;
        obj["email"] = email;
        obj["password"] = password;
        return obj;
    }

    static Manager fromJson(const QJsonObject& obj) {
        Manager m;
        m.id = obj["id"].toInt();
        m.fname = obj["fname"].toString();
        m.lname = obj["lname"].toString();
        m.email = obj["email"].toString();
        m.password = obj["password"].toString();
        return m;
    }
};

struct Admin {
    int id = -1;
    QString username;
    QString email;
    QString password;

    QString fullName() const { return username; }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["username"] = username;
        obj["email"] = email;
        obj["password"] = password;
        return obj;
    }

    static Admin fromJson(const QJsonObject &obj) {
        Admin a;
        a.id = obj["id"].toInt(-1);
        a.username = obj["username"].toString();
        a.email = obj["email"].toString();
        a.password = obj["password"].toString();
        return a;
    }
};

struct InvitationCode {
    int id;
    int id_parent;
    QString code; // 6-символный код
    QDateTime created_at;
    bool used = false;
    int id_invited = -1; // ID приглашённого пользователя

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["id_parent"] = id_parent;
        obj["code"] = code;
        obj["created_at"] = created_at.toString("yyyy-MM-ddTHH:mm:ss");
        obj["used"] = used;
        obj["id_invited"] = id_invited;
        return obj;
    }

    static InvitationCode fromJson(const QJsonObject& obj) {
        InvitationCode ic;
        ic.id = obj["id"].toInt();
        ic.id_parent = obj["id_parent"].toInt();
        ic.code = obj["code"].toString();
        ic.created_at = QDateTime::fromString(obj["created_at"].toString(), "yyyy-MM-ddTHH:mm:ss");
        ic.used = obj["used"].toBool(false);
        ic.id_invited = obj["id_invited"].toInt(-1);
        return ic;
    }
};

#endif // MODELS_H
