#include "datamanager.h"
#include "models.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDebug>
#include <cstdlib>
#include <ctime>

DataManager::DataManager(const QString& requestedPath) {
    // Resolve the data path: prefer the requested path, otherwise try
    // several sensible fallbacks so the app works when run from build dirs.
    QStringList candidates;

    if (!requestedPath.isEmpty()) candidates << requestedPath;

    // Common locations relative to the application binary
    // Prefer build-local `data/` (appDir/data) so when running from a build
    // directory the build's data is chosen. Fallback to project `../data` if
    // build data is not present. Also check current working directory.
    QString appDir = QCoreApplication::applicationDirPath();
    candidates << appDir + "/data";          // e.g. build/data or Desktop-Debug/data
    // candidates << appDir + "/../data";      // project/data
    candidates << QDir::currentPath() + "/data"; // cwd/data
    // candidates << QDir(appDir).absolutePath() + "/../data"; // absolute project/data

    // Try to find the first existing candidate
    QString found;
    for (const QString &c : candidates) {
        QDir dir(c);
        if (dir.exists()) {
            found = QDir(dir).canonicalPath();
            break;
        }
    }

    if (!found.isEmpty()) {
        dataPath = found;
        qDebug() << "DataManager: using data directory:" << dataPath;
        QDir dir(dataPath);
        QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
        qDebug() << "Data files:" << files;
    } else {
        // As a last resort, use provided string as-is (may be relative)
        dataPath = requestedPath.isEmpty() ? "data" : requestedPath;
        qWarning() << "DataManager: could not find a data directory among candidates.";
        qWarning() << "Tried:" << candidates;
        qWarning() << "Falling back to" << dataPath << "(may fail to open files).";
    }
}

QJsonArray DataManager::loadJson(const QString& filename) const {
    QString filePath = QDir(dataPath).filePath(filename);
    QFile file(filePath);

    if (!file.exists()) {
        qWarning() << "File not found at" << filePath << "- attempting fallbacks";

        // Try fallback locations: relative filename, appDir/data, cwd/data
        QString appDir = QCoreApplication::applicationDirPath();
        QString fallback1 = QDir(appDir).filePath("../data/" + filename);
        QString fallback2 = QDir::currentPath() + "/data/" + filename;

        if (QFile::exists(fallback1)) {
            file.setFileName(fallback1);
            qDebug() << "Found file at fallback:" << fallback1;
        } else if (QFile::exists(fallback2)) {
            file.setFileName(fallback2);
            qDebug() << "Found file at fallback:" << fallback2;
        } else {
            qWarning() << "File not found in fallbacks either:" << fallback1 << fallback2;
            return QJsonArray();
        }
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << file.fileName() << "Error:" << file.errorString();
        return QJsonArray();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        qDebug() << "Loaded" << filename << "with" << doc.array().size() << "items";
        return doc.array();
    }
    qWarning() << "Invalid JSON in" << filename;
    return QJsonArray();
}

void DataManager::saveJson(const QString& filename, const QJsonArray& data) {
    QString filePath = dataPath + "/" + filename;
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write to file:" << filePath;
        return;
    }

    QJsonDocument doc(data);
    file.write(doc.toJson());
    file.close();
}

// Patient operations
QList<Patient> DataManager::getAllPatients() const {
    QList<Patient> patients;
    QJsonArray array = loadJson("patient.json");

    for (const QJsonValue& value : array) {
        patients.append(Patient::fromJson(value.toObject()));
    }
    return patients;
}

Patient DataManager::getPatientById(int id) const {
    QJsonArray array = loadJson("patient.json");

    for (const QJsonValue& value : array) {
        Patient p = Patient::fromJson(value.toObject());
        if (p.id_patient == id) {
            return p;
        }
    }
    return Patient();
}

void DataManager::addPatient(const Patient& patient) {
    QJsonArray array = loadJson("patient.json");
    array.append(patient.toJson());
    saveJson("patient.json", array);
}

void DataManager::updatePatient(const Patient& patient) {
    QJsonArray array = loadJson("patient.json");

    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array[i].toObject();
        if (obj["id_patient"].toInt() == patient.id_patient) {
            array[i] = patient.toJson();
            saveJson("patient.json", array);
            return;
        }
    }
}

void DataManager::deletePatient(int id) {
    QJsonArray array = loadJson("patient.json");

    for (int i = 0; i < array.size(); ++i) {
        if (array[i].toObject()["id_patient"].toInt() == id) {
            array.removeAt(i);
            saveJson("patient.json", array);

            // Also remove family relations
            QJsonArray groupArray = loadJson("patient_group.json");
            for (int j = groupArray.size() - 1; j >= 0; --j) {
                QJsonObject obj = groupArray[j].toObject();
                if (obj["id_parent"].toInt() == id || obj["id_child"].toInt() == id) {
                    groupArray.removeAt(j);
                }
            }
            saveJson("patient_group.json", groupArray);

            // Also remove appointments
            QJsonArray apArray = loadJson("appointment.json");
            for (int j = apArray.size() - 1; j >= 0; --j) {
                if (apArray[j].toObject()["id_patient"].toInt() == id) {
                    int apId = apArray[j].toObject()["id_ap"].toInt();
                    apArray.removeAt(j);

                    // Remove recipe if exists
                    QJsonArray recipeArray = loadJson("recipe.json");
                    for (int k = recipeArray.size() - 1; k >= 0; --k) {
                        if (recipeArray[k].toObject()["id_ap"].toInt() == apId) {
                            recipeArray.removeAt(k);
                        }
                    }
                    saveJson("recipe.json", recipeArray);
                }
            }
            saveJson("appointment.json", apArray);
            return;
        }
    }
}

bool DataManager::patientExists(int id) const {
    QJsonArray array = loadJson("patient.json");
    for (const QJsonValue& value : array) {
        if (value.toObject()["id_patient"].toInt() == id) {
            return true;
        }
    }
    return false;
}

bool DataManager::emailExists(const QString& email) const {
    QJsonArray array = loadJson("patient.json");
    for (const QJsonValue& value : array) {
        if (value.toObject()["email"].toString() == email) {
            return true;
        }
    }
    return false;
}

bool DataManager::snilsExists(const QString& snils) const {
    QJsonArray array = loadJson("patient.json");
    for (const QJsonValue& value : array) {
        if (value.toObject()["snils"].toString() == snils) {
            return true;
        }
    }
    return false;
}

bool DataManager::omsExists(const QString& oms) const {
    QJsonArray array = loadJson("patient.json");
    for (const QJsonValue& value : array) {
        if (value.toObject()["oms"].toString() == oms) {
            return true;
        }
    }
    return false;
}

int DataManager::getNextPatientId() const {
    QJsonArray array = loadJson("patient.json");
    int maxId = 0;

    for (const QJsonValue& value : array) {
        int id = value.toObject()["id_patient"].toInt();
        if (id > maxId) {
            maxId = id;
        }
    }
    return maxId + 1;
}

// Doctor operations
QList<Doctor> DataManager::getAllDoctors() const {
    QList<Doctor> doctors;
    QJsonArray array = loadJson("doctor.json");

    for (const QJsonValue& value : array) {
        doctors.append(Doctor::fromJson(value.toObject()));
    }
    return doctors;
}

Doctor DataManager::getDoctorById(int id) const {
    QJsonArray array = loadJson("doctor.json");

    for (const QJsonValue& value : array) {
        Doctor d = Doctor::fromJson(value.toObject());
        if (d.id_doctor == id) {
            return d;
        }
    }
    return Doctor();
}

// Specialization operations
QList<Specialization> DataManager::getAllSpecializations() const {
    QList<Specialization> specs;
    QJsonArray array = loadJson("specialization.json");

    for (const QJsonValue& value : array) {
        specs.append(Specialization::fromJson(value.toObject()));
    }
    return specs;
}

Specialization DataManager::getSpecializationById(int id) const {
    QJsonArray array = loadJson("specialization.json");

    for (const QJsonValue& value : array) {
        Specialization s = Specialization::fromJson(value.toObject());
        if (s.id_spec == id) {
            return s;
        }
    }
    return Specialization();
}

// Room operations
QList<Room> DataManager::getAllRooms() const {
    QList<Room> rooms;
    QJsonArray array = loadJson("room.json");

    for (const QJsonValue& value : array) {
        rooms.append(Room::fromJson(value.toObject()));
    }
    return rooms;
}

Room DataManager::getRoomById(int id) const {
    QJsonArray array = loadJson("room.json");

    for (const QJsonValue& value : array) {
        Room r = Room::fromJson(value.toObject());
        if (r.id_room == id) {
            return r;
        }
    }
    return Room();
}

// Appointment operations
QList<Appointment> DataManager::getAllAppointments() const {
    QList<Appointment> appointments;
    QJsonArray array = loadJson("appointment.json");

    for (const QJsonValue& value : array) {
        appointments.append(Appointment::fromJson(value.toObject()));
    }
    return appointments;
}

QList<Appointment> DataManager::getPatientAppointments(int patientId) const {
    QList<Appointment> appointments;
    QJsonArray array = loadJson("appointment.json");

    for (const QJsonValue& value : array) {
        Appointment a = Appointment::fromJson(value.toObject());
        if (a.id_patient == patientId) {
            appointments.append(a);
        }
    }
    return appointments;
}

Appointment DataManager::getAppointmentById(int id) const {
    QJsonArray array = loadJson("appointment.json");

    for (const QJsonValue& value : array) {
        Appointment a = Appointment::fromJson(value.toObject());
        if (a.id_ap == id) {
            return a;
        }
    }
    return Appointment();
}

void DataManager::addAppointment(const Appointment& appointment) {
    QJsonArray array = loadJson("appointment.json");
    array.append(appointment.toJson());
    saveJson("appointment.json", array);
}

void DataManager::updateAppointment(const Appointment& appointment) {
    QJsonArray array = loadJson("appointment.json");

    for (int i = 0; i < array.size(); ++i) {
        QJsonObject obj = array[i].toObject();
        if (obj["id_ap"].toInt() == appointment.id_ap) {
            array[i] = appointment.toJson();
            saveJson("appointment.json", array);
            return;
        }
    }
}

void DataManager::deleteAppointment(int id) {
    QJsonArray array = loadJson("appointment.json");

    for (int i = 0; i < array.size(); ++i) {
        if (array[i].toObject()["id_ap"].toInt() == id) {
            array.removeAt(i);
            saveJson("appointment.json", array);

            // Also remove recipe if exists
            QJsonArray recipeArray = loadJson("recipe.json");
            for (int j = recipeArray.size() - 1; j >= 0; --j) {
                if (recipeArray[j].toObject()["id_ap"].toInt() == id) {
                    recipeArray.removeAt(j);
                }
            }
            saveJson("recipe.json", recipeArray);
            return;
        }
    }
}

int DataManager::getNextAppointmentId() const {
    QJsonArray array = loadJson("appointment.json");
    int maxId = 0;

    for (const QJsonValue& value : array) {
        int id = value.toObject()["id_ap"].toInt();
        if (id > maxId) {
            maxId = id;
        }
    }
    return maxId + 1;
}

// Appointment Schedule operations
QList<AppointmentSchedule> DataManager::getAllSchedules() const {
    QList<AppointmentSchedule> schedules;
    QJsonArray array = loadJson("appointment_schedule.json");

    for (const QJsonValue& value : array) {
        schedules.append(AppointmentSchedule::fromJson(value.toObject()));
    }
    return schedules;
}

QList<AppointmentSchedule> DataManager::getDoctorSchedules(int doctorId) const {
    QList<AppointmentSchedule> schedules;
    QJsonArray array = loadJson("appointment_schedule.json");

    for (const QJsonValue& value : array) {
        AppointmentSchedule s = AppointmentSchedule::fromJson(value.toObject());
        if (s.id_doctor == doctorId) {
            schedules.append(s);
        }
    }
    return schedules;
}

QList<AppointmentSchedule> DataManager::getAvailableSchedules(int doctorId) const {
    QList<AppointmentSchedule> available;
    QList<AppointmentSchedule> schedules = getDoctorSchedules(doctorId);
    QDateTime now = QDateTime::currentDateTime();

    qDebug() << "getAvailableSchedules for doctor" << doctorId << "- Total schedules:" << schedules.size();
    
    for (const AppointmentSchedule& schedule : schedules) {
        qDebug() << "  Schedule" << schedule.id_ap_sch << ":" 
                 << schedule.time_from.toString("yyyy-MM-dd HH:mm") 
                 << "Status:" << schedule.status
                 << "isPast:" << (schedule.time_from < now);
        
        // Пропускаем прошедшие слоты
        if (schedule.time_from < now) {
            continue;
        }

        // Если для этого слота уже есть запись (даже если статус не обновлён), исключаем его.
        // Сравниваем по врачу и точному времени начала.
        bool occupiedByAppointment = false;
        QList<Appointment> appts = getAppointmentsByDoctor(schedule.id_doctor);
        for (const Appointment &a : appts) {
            if (a.date.isValid() && schedule.time_from.isValid() && a.date == schedule.time_from) {
                occupiedByAppointment = true;
                qDebug() << "    Excluding schedule" << schedule.id_ap_sch << "because appointment" << a.id_ap << "exists at same time";
                break;
            }
        }
        if (occupiedByAppointment) continue;

        // Возвращаем только свободные слоты (по умолчанию "free" если не указано)
        QString st = schedule.status.trimmed().toLower();
        if (st.isEmpty() || st == "free" || st == "available") {
            available.append(schedule);
        }
    }
    
    qDebug() << "Available schedules count:" << available.size();
    return available;
}

// Patient Group operations
QList<PatientGroup> DataManager::getPatientFamilyMembers(int parentId) const {
    QList<PatientGroup> members;
    QJsonArray array = loadJson("patient_group.json");

    for (const QJsonValue& value : array) {
        PatientGroup pg = PatientGroup::fromJson(value.toObject());
        if (pg.id_parent == parentId) {
            members.append(pg);
        }
    }
    return members;
}

QList<PatientGroup> DataManager::getPatientParents(int childId) const {
    QList<PatientGroup> parents;
    QJsonArray array = loadJson("patient_group.json");

    for (const QJsonValue& value : array) {
        PatientGroup pg = PatientGroup::fromJson(value.toObject());
        if (pg.id_child == childId) {
            parents.append(pg);
        }
    }
    return parents;
}

void DataManager::addFamilyMember(const PatientGroup& group) {
    QJsonArray array = loadJson("patient_group.json");
    array.append(group.toJson());
    saveJson("patient_group.json", array);
}

void DataManager::updateFamilyGroup(const PatientGroup& group) {
    QJsonArray array = loadJson("patient_group.json");

    for (int i = 0; i < array.size(); ++i) {
        if (array[i].toObject()["id_patient_group"].toInt() == group.id_patient_group) {
            array[i] = group.toJson();
            saveJson("patient_group.json", array);
            return;
        }
    }
}

void DataManager::removeFamilyMember(int id_patient_group) {
    QJsonArray array = loadJson("patient_group.json");

    for (int i = 0; i < array.size(); ++i) {
        if (array[i].toObject()["id_patient_group"].toInt() == id_patient_group) {
            array.removeAt(i);
            saveJson("patient_group.json", array);
            return;
        }
    }
}

bool DataManager::isFamilyMember(int parentId, int childId) const {
    QJsonArray array = loadJson("patient_group.json");

    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        if (obj["id_parent"].toInt() == parentId &&
            obj["id_child"].toInt() == childId) {
            return true;
        }
    }
    return false;
}

bool DataManager::isPatientInAnyFamily(int patientId) const {
    QJsonArray array = loadJson("patient_group.json");

    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        // Проверяем, состоит ли пациент в семье как parent или как child
        if (obj["id_parent"].toInt() == patientId || obj["id_child"].toInt() == patientId) {
            return true;
        }
    }
    return false;
}

int DataManager::getNextPatientGroupId() const {
    QJsonArray array = loadJson("patient_group.json");
    int maxId = 0;

    for (const QJsonValue& value : array) {
        int id = value.toObject()["id_patient_group"].toInt();
        if (id > maxId) {
            maxId = id;
        }
    }
    return maxId + 1;
}

// Recipe operations
QList<Recipe> DataManager::getAllRecipes() const {
    QList<Recipe> recipes;
    QJsonArray array = loadJson("recipe.json");

    for (const QJsonValue& value : array) {
        recipes.append(Recipe::fromJson(value.toObject()));
    }
    return recipes;
}

Recipe DataManager::getRecipeByAppointmentId(int appointmentId) const {
    QJsonArray array = loadJson("recipe.json");

    for (const QJsonValue& value : array) {
        Recipe r = Recipe::fromJson(value.toObject());
        if (r.id_ap == appointmentId) {
            return r;
        }
    }
    return Recipe();
}

void DataManager::addRecipe(const Recipe& recipe) {
    QJsonArray array = loadJson("recipe.json");
    array.append(recipe.toJson());
    saveJson("recipe.json", array);
}

int DataManager::getNextRecipeId() const {
    QJsonArray array = loadJson("recipe.json");
    int maxId = 0;
    for (const QJsonValue& value : array) {
        int id = value.toObject()["id"].toInt();
        if (id > maxId) maxId = id;
    }
    return maxId + 1;
}

// Diagnosis operations
QList<Diagnosis> DataManager::getAllDiagnoses() const {
    QList<Diagnosis> diagnoses;
    QJsonArray array = loadJson("diagnosis.json");

    for (const QJsonValue& value : array) {
        diagnoses.append(Diagnosis::fromJson(value.toObject()));
    }
    return diagnoses;
}

Diagnosis DataManager::getDiagnosisById(int id) const {
    QJsonArray array = loadJson("diagnosis.json");

    for (const QJsonValue& value : array) {
        Diagnosis d = Diagnosis::fromJson(value.toObject());
        if (d.id_diagnosis == id) {
            return d;
        }
    }
    return Diagnosis();
}

void DataManager::addDiagnosis(const Diagnosis& diagnosis) {
    QJsonArray array = loadJson("diagnosis.json");
    array.append(diagnosis.toJson());
    saveJson("diagnosis.json", array);
}

int DataManager::getNextDiagnosisId() const {
    QJsonArray array = loadJson("diagnosis.json");
    int maxId = 0;
    for (const QJsonValue& value : array) {
        int id = value.toObject()["id_diagnosis"].toInt();
        if (id > maxId) maxId = id;
    }
    return maxId + 1;
}

QList<Appointment> DataManager::getAppointmentsByDoctor(int doctorId) const {
    QList<Appointment> appointments;
    QJsonArray array = loadJson("appointment.json");

    for (const QJsonValue& value : array) {
        Appointment apt = Appointment::fromJson(value.toObject());
        if (apt.id_doctor == doctorId) {
            appointments.append(apt);
        }
    }
    return appointments;
}

QList<AppointmentSchedule> DataManager::getSchedulesByRoom(int roomId) const {
    QList<AppointmentSchedule> schedules;
    QJsonArray array = loadJson("appointment_schedule.json");

    for (const QJsonValue& value : array) {
        AppointmentSchedule s = AppointmentSchedule::fromJson(value.toObject());
        if (s.id_room == roomId) {
            schedules.append(s);
        }
    }
    return schedules;
}

bool DataManager::doctorExists(int id) const {
    QJsonArray array = loadJson("doctor.json");

    for (const QJsonValue& value : array) {
        Doctor d = Doctor::fromJson(value.toObject());
        if (d.id_doctor == id) {
            return true;
        }
    }
    return false;
}

QList<Manager> DataManager::getAllManagers() const {
    QList<Manager> managers;
    QJsonArray array = loadJson("manager.json");

    for (const QJsonValue& value : array) {
        managers.append(Manager::fromJson(value.toObject()));
    }
    return managers;
}

Manager DataManager::getManagerById(int id) const {
    QJsonArray array = loadJson("manager.json");

    for (const QJsonValue& value : array) {
        Manager m = Manager::fromJson(value.toObject());
        if (m.id == id) {
            return m;
        }
    }
    return Manager();
}

bool DataManager::managerExists(int id) const {
    QJsonArray array = loadJson("manager.json");

    for (const QJsonValue& value : array) {
        Manager m = Manager::fromJson(value.toObject());
        if (m.id == id) {
            return true;
        }
    }
    return false;
}

bool DataManager::managerLogin(int id, const QString& password) const {
    QJsonArray array = loadJson("manager.json");

    for (const QJsonValue& value : array) {
        Manager m = Manager::fromJson(value.toObject());
        if (m.id == id && verifyPassword(password, m.password)) {
            return true;
        }
    }
    return false;
}

void DataManager::addManager(const Manager& manager) {
    QJsonArray array = loadJson("manager.json");
    array.append(manager.toJson());
    saveJson("manager.json", array);
}

void DataManager::updateManager(const Manager& manager) {
    QJsonArray array = loadJson("manager.json");
    for (int i = 0; i < array.size(); ++i) {
        Manager m = Manager::fromJson(array[i].toObject());
        if (m.id == manager.id) {
            array[i] = manager.toJson();
            break;
        }
    }
    saveJson("manager.json", array);
}

void DataManager::deleteManager(int id) {
    QJsonArray array = loadJson("manager.json");
    QJsonArray newArray;
    for (const QJsonValue& value : array) {
        Manager m = Manager::fromJson(value.toObject());
        if (m.id != id) {
            newArray.append(value);
        }
    }
    saveJson("manager.json", newArray);
}

int DataManager::getNextManagerId() const {
    QJsonArray array = loadJson("manager.json");
    int maxId = 0;
    for (const QJsonValue& value : array) {
        Manager m = Manager::fromJson(value.toObject());
        if (m.id > maxId) maxId = m.id;
    }
    return maxId + 1;
}

// Admin Doctor operations
void DataManager::addDoctor(const Doctor& doctor) {
    QJsonArray array = loadJson("doctor.json");
    array.append(doctor.toJson());
    saveJson("doctor.json", array);
}

void DataManager::updateDoctor(const Doctor& doctor) {
    QJsonArray array = loadJson("doctor.json");
    for (int i = 0; i < array.size(); ++i) {
        Doctor d = Doctor::fromJson(array[i].toObject());
        if (d.id_doctor == doctor.id_doctor) {
            array[i] = doctor.toJson();
            break;
        }
    }
    saveJson("doctor.json", array);
}

void DataManager::deleteDoctor(int id) {
    QJsonArray array = loadJson("doctor.json");
    QJsonArray newArray;
    for (const QJsonValue& value : array) {
        Doctor d = Doctor::fromJson(value.toObject());
        if (d.id_doctor != id) {
            newArray.append(value);
        }
    }
    saveJson("doctor.json", newArray);
}

int DataManager::getNextDoctorId() const {
    QJsonArray array = loadJson("doctor.json");
    int maxId = 0;
    for (const QJsonValue& value : array) {
        Doctor d = Doctor::fromJson(value.toObject());
        if (d.id_doctor > maxId) maxId = d.id_doctor;
    }
    return maxId + 1;
}

// Admin Schedule operations
AppointmentSchedule DataManager::getScheduleById(int id) const {
    QJsonArray array = loadJson("appointment_schedule.json");
    for (const QJsonValue& value : array) {
        AppointmentSchedule s = AppointmentSchedule::fromJson(value.toObject());
        if (s.id_ap_sch == id) {
            return s;
        }
    }
    return AppointmentSchedule();
}

bool DataManager::canAddSchedule(const AppointmentSchedule& schedule) const {
    // Validate that the new schedule does not overlap with existing schedules
    // for the same doctor or in the same room. Touching endpoints are allowed.
    QList<AppointmentSchedule> doctorSchedules = getDoctorSchedules(schedule.id_doctor);
    for (const AppointmentSchedule &s : doctorSchedules) {
        if (s.time_from.isValid() && s.time_to.isValid()) {
            if (schedule.time_from < s.time_to && s.time_from < schedule.time_to) {
                return false;
            }
        }
    }

    QList<AppointmentSchedule> allSchedules = getAllSchedules();
    for (const AppointmentSchedule &s : allSchedules) {
        if (s.id_room == schedule.id_room && s.time_from.isValid() && s.time_to.isValid()) {
            if (schedule.time_from < s.time_to && s.time_from < schedule.time_to) {
                return false;
            }
        }
    }

    return true;
}

void DataManager::addSchedule(const AppointmentSchedule& schedule) {
    if (!canAddSchedule(schedule)) {
        qWarning() << "addSchedule: rejected overlapping schedule for doctor" << schedule.id_doctor << "or room" << schedule.id_room;
        return;
    }

    QJsonArray array = loadJson("appointment_schedule.json");
    array.append(schedule.toJson());
    saveJson("appointment_schedule.json", array);
}

void DataManager::updateSchedule(const AppointmentSchedule& schedule) {
    QJsonArray array = loadJson("appointment_schedule.json");
    for (int i = 0; i < array.size(); ++i) {
        AppointmentSchedule s = AppointmentSchedule::fromJson(array[i].toObject());
        if (s.id_ap_sch == schedule.id_ap_sch) {
            array[i] = schedule.toJson();
            break;
        }
    }
    saveJson("appointment_schedule.json", array);
}

void DataManager::deleteSchedule(int id) {
    QJsonArray array = loadJson("appointment_schedule.json");
    QJsonArray newArray;
    for (const QJsonValue& value : array) {
        AppointmentSchedule s = AppointmentSchedule::fromJson(value.toObject());
        if (s.id_ap_sch != id) {
            newArray.append(value);
        }
    }
    saveJson("appointment_schedule.json", newArray);
}

int DataManager::getNextScheduleId() const {
    QJsonArray array = loadJson("appointment_schedule.json");
    int maxId = 0;
    for (const QJsonValue& value : array) {
        AppointmentSchedule s = AppointmentSchedule::fromJson(value.toObject());
        if (s.id_ap_sch > maxId) maxId = s.id_ap_sch;
    }
    return maxId + 1;
}

// Admin Specialization operations
void DataManager::addSpecialization(const Specialization& spec) {
    QJsonArray array = loadJson("specialization.json");
    array.append(spec.toJson());
    saveJson("specialization.json", array);
}

void DataManager::deleteSpecialization(int id) {
    QJsonArray array = loadJson("specialization.json");
    QJsonArray newArray;
    for (const QJsonValue& value : array) {
        Specialization s = Specialization::fromJson(value.toObject());
        if (s.id_spec != id) {
            newArray.append(value);
        }
    }
    saveJson("specialization.json", newArray);
}

int DataManager::getNextSpecializationId() const {
    QJsonArray array = loadJson("specialization.json");
    int maxId = 0;
    for (const QJsonValue& value : array) {
        Specialization s = Specialization::fromJson(value.toObject());
        if (s.id_spec > maxId) maxId = s.id_spec;
    }
    return maxId + 1;
}

// Admin Room operations
void DataManager::addRoom(const Room& room) {
    QJsonArray array = loadJson("room.json");
    array.append(room.toJson());
    saveJson("room.json", array);
}

void DataManager::deleteRoom(int id) {
    QJsonArray array = loadJson("room.json");
    QJsonArray newArray;
    for (const QJsonValue& value : array) {
        Room r = Room::fromJson(value.toObject());
        if (r.id_room != id) {
            newArray.append(value);
        }
    }
    saveJson("room.json", newArray);
}

int DataManager::getNextRoomId() const {
    QJsonArray array = loadJson("room.json");
    int maxId = 0;
    for (const QJsonValue& value : array) {
        Room r = Room::fromJson(value.toObject());
        if (r.id_room > maxId) maxId = r.id_room;
    }
    return maxId + 1;
}

// Admin Diagnosis operations
void DataManager::deleteDiagnosis(int id) {
    QJsonArray array = loadJson("diagnosis.json");
    QJsonArray newArray;
    for (const QJsonValue& value : array) {
        Diagnosis d = Diagnosis::fromJson(value.toObject());
        if (d.id_diagnosis != id) {
            newArray.append(value);
        }
    }
    saveJson("diagnosis.json", newArray);
}

// Admin login
bool DataManager::adminLogin(int id, const QString& password) const {
    QJsonArray array = loadJson("admin.json");
    for (const QJsonValue& value : array) {
        QJsonObject obj = value.toObject();
        if (obj["id"].toInt() == id && verifyPassword(password, obj["password"].toString())) {
            return true;
        }
    }
    return false;
}

// Admin helpers
QList<Admin> DataManager::getAllAdmins() const {
    QList<Admin> list;
    QJsonArray array = loadJson("admin.json");
    for (const QJsonValue &v : array) {
        list.append(Admin::fromJson(v.toObject()));
    }
    return list;
}

Admin DataManager::getAdminById(int id) const {
    QJsonArray array = loadJson("admin.json");
    for (const QJsonValue &v : array) {
        Admin a = Admin::fromJson(v.toObject());
        if (a.id == id) return a;
    }
    return Admin();
}

Admin DataManager::getAdminByEmail(const QString &email) const {
    QJsonArray array = loadJson("admin.json");
    for (const QJsonValue &v : array) {
        Admin a = Admin::fromJson(v.toObject());
        if (a.email == email) return a;
    }
    return Admin();
}

bool DataManager::adminLoginByEmail(const QString &email, const QString &password) const {
    Admin a = getAdminByEmail(email);
    if (a.id <= 0) return false;
    return verifyPassword(password, a.password);
}

// CHANGED: Add updateAdmin method
void DataManager::updateAdmin(const Admin& admin) {
    QJsonArray array = loadJson("admin.json");
    QJsonArray newArray;
    bool found = false;
    
    for (const QJsonValue &v : array) {
        QJsonObject obj = v.toObject();
        if (obj["id"].toInt() == admin.id) {
            newArray.append(admin.toJson());
            found = true;
        } else {
            newArray.append(obj);
        }
    }
    
    if (found) {
        saveJson("admin.json", newArray);
    }
}

// Authentication by email + password
bool DataManager::patientLoginByEmail(const QString& email, const QString& password) const {
    const QList<Patient>& patients = getAllPatients();
    qDebug() << "Checking" << patients.size() << "patients for email:" << email;
    for (const Patient& p : patients) {
        if (p.email == email) {
            qDebug() << "Found patient with email:" << email;
            bool verified = verifyPassword(password, p.password);
            qDebug() << "Password verification result:" << verified;
            if (verified) {
                return true;
            }
        }
    }
    return false;
}

bool DataManager::doctorLoginByEmail(const QString& email, const QString& password) const {
    const QList<Doctor>& doctors = getAllDoctors();
    for (const Doctor& d : doctors) {
        if (d.email == email && verifyPassword(password, d.password)) {
            return true;
        }
    }
    return false;
}

bool DataManager::managerLoginByEmail(const QString& email, const QString& password) const {
    const QList<Manager>& managers = getAllManagers();
    for (const Manager& m : managers) {
        if (m.email == email && verifyPassword(password, m.password)) {
            return true;
        }
    }
    return false;
}

Patient DataManager::getPatientByEmail(const QString& email) const {
    const QList<Patient>& patients = getAllPatients();
    for (const Patient& p : patients) {
        if (p.email == email) {
            return p;
        }
    }
    return Patient();
}

Doctor DataManager::getDoctorByEmail(const QString& email) const {
    const QList<Doctor>& doctors = getAllDoctors();
    for (const Doctor& d : doctors) {
        if (d.email == email) {
            return d;
        }
    }
    return Doctor();
}

Manager DataManager::getManagerByEmail(const QString& email) const {
    const QList<Manager>& managers = getAllManagers();
    for (const Manager& m : managers) {
        if (m.email == email) {
            return m;
        }
    }
    return Manager();
}

QString DataManager::generateInvitationCode(int parentId) {
    // Генерируем уникальный 6-символный код
    QJsonArray codes = loadJson("invitation_code.json");
    QString code;
    bool codeExists = true;
    
    // Генерируем уникальный код
    while (codeExists) {
        code.clear();
        const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        QRandomGenerator gen = QRandomGenerator::securelySeeded();
        for (int i = 0; i < 6; ++i) {
            int randomIdx = gen.bounded(0, chars.length());
            code += chars[randomIdx];
        }
        
        // Проверяем, существует ли уже такой код
        codeExists = false;
        for (const QJsonValue& v : codes) {
            InvitationCode ic = InvitationCode::fromJson(v.toObject());
            if (ic.code == code) {
                codeExists = true;
                break;
            }
        }
    }

    // Создаём запись о коде
    InvitationCode ic;
    ic.id = getNextInvitationCodeId();
    ic.id_parent = parentId;
    ic.code = code;
    ic.created_at = QDateTime::currentDateTime();
    ic.used = false;
    ic.id_invited = -1;

    // Сохраняем код
    codes.append(ic.toJson());
    saveJson("invitation_code.json", codes);

    return code;
}

QList<InvitationCode> DataManager::getInvitationCodes(int parentId) const {
    QList<InvitationCode> result;
    QJsonArray codes = loadJson("invitation_code.json");
    for (const QJsonValue& v : codes) {
        InvitationCode ic = InvitationCode::fromJson(v.toObject());
        if (ic.id_parent == parentId) {
            result.append(ic);
        }
    }
    return result;
}

InvitationCode DataManager::getInvitationCodeByCode(const QString& code) const {
    QJsonArray codes = loadJson("invitation_code.json");
    for (const QJsonValue& v : codes) {
        InvitationCode ic = InvitationCode::fromJson(v.toObject());
        if (ic.code == code) {
            return ic;
        }
    }
    return InvitationCode();
}

void DataManager::useInvitationCode(const QString& code, int invitedUserId) {
    QJsonArray codes = loadJson("invitation_code.json");
    for (int i = 0; i < codes.size(); ++i) {
        InvitationCode ic = InvitationCode::fromJson(codes[i].toObject());
        if (ic.code == code) {
            ic.used = true;
            ic.id_invited = invitedUserId;
            codes[i] = ic.toJson();
            break;
        }
    }
    saveJson("invitation_code.json", codes);
}

int DataManager::getNextInvitationCodeId() const {
    QJsonArray codes = loadJson("invitation_code.json");
    int maxId = 0;
    for (const QJsonValue& v : codes) {
        int id = v.toObject()["id"].toInt();
        if (id > maxId) maxId = id;
    }
    return maxId + 1;
}

