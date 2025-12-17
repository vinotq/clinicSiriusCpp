#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QString>
#include <QList>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "models.h"

class DataManager {
public:
    // If dataPath is empty or doesn't exist, DataManager will try common
    // locations (application dir, parent dir, current working dir) to
    // locate the `data/` folder.
    DataManager(const QString& dataPath = QString());
    
    // Patient operations
    QList<Patient> getAllPatients() const;
    Patient getPatientById(int id) const;
    void addPatient(const Patient& patient);
    void updatePatient(const Patient& patient);
    void deletePatient(int id);
    bool patientExists(int id) const;
    bool emailExists(const QString& email) const;
    bool snilsExists(const QString& snils) const;
    bool omsExists(const QString& oms) const;
    int getNextPatientId() const;
    
    // Doctor operations
    QList<Doctor> getAllDoctors() const;
    Doctor getDoctorById(int id) const;
    
    // Specialization operations
    QList<Specialization> getAllSpecializations() const;
    Specialization getSpecializationById(int id) const;
    
    // Room operations
    QList<Room> getAllRooms() const;
    Room getRoomById(int id) const;
    
    // Appointment operations
    QList<Appointment> getAllAppointments() const;
    QList<Appointment> getPatientAppointments(int patientId) const;
    Appointment getAppointmentById(int id) const;
    void addAppointment(const Appointment& appointment);
    void updateAppointment(const Appointment& appointment);
    void deleteAppointment(int id);
    int getNextAppointmentId() const;
    
    // Appointment Schedule operations
    QList<AppointmentSchedule> getAllSchedules() const;
    QList<AppointmentSchedule> getDoctorSchedules(int doctorId) const;
    QList<AppointmentSchedule> getAvailableSchedules(int doctorId) const;
    QList<AppointmentSchedule> getSchedulesByRoom(int roomId) const;
    QList<Appointment> getAppointmentsByDoctor(int doctorId) const;
    
    // Patient Group operations
    QList<PatientGroup> getPatientFamilyMembers(int parentId) const;
    QList<PatientGroup> getPatientParents(int childId) const;
    void addFamilyMember(const PatientGroup& group);
    void updateFamilyGroup(const PatientGroup& group);
    void removeFamilyMember(int id_patient_group);
    bool isFamilyMember(int parentId, int childId) const;
    bool isPatientInAnyFamily(int patientId) const;
    int getNextPatientGroupId() const;
    
    // Diagnosis operations
    QList<Diagnosis> getAllDiagnoses() const;
    Diagnosis getDiagnosisById(int id) const;
    void addDiagnosis(const Diagnosis& diagnosis);
    int getNextDiagnosisId() const;
    
    // Recipe operations
    QList<Recipe> getAllRecipes() const;
    Recipe getRecipeByAppointmentId(int appointmentId) const;
    void addRecipe(const Recipe& recipe);
    int getNextRecipeId() const;
    
    // Doctor login
    bool doctorExists(int id) const;
    
    // Manager operations
    QList<Manager> getAllManagers() const;
    Manager getManagerById(int id) const;
    bool managerExists(int id) const;
    bool managerLogin(int id, const QString& password) const;
    void addManager(const Manager& manager);
    void updateManager(const Manager& manager);
    void deleteManager(int id);
    int getNextManagerId() const;
    
    // Admin Doctor operations
    void addDoctor(const Doctor& doctor);
    void updateDoctor(const Doctor& doctor);
    void deleteDoctor(int id);
    int getNextDoctorId() const;
    
    // Admin Schedule operations
    AppointmentSchedule getScheduleById(int id) const;
    void addSchedule(const AppointmentSchedule& schedule);
    // Check if schedule can be added (no overlap with existing schedules for same doctor or room).
    bool canAddSchedule(const AppointmentSchedule& schedule) const;
    void updateSchedule(const AppointmentSchedule& schedule);
    void deleteSchedule(int id);
    int getNextScheduleId() const;
    
    // Admin Specialization operations
    void addSpecialization(const Specialization& spec);
    void deleteSpecialization(int id);
    int getNextSpecializationId() const;
    
    // Admin Room operations
    void addRoom(const Room& room);
    void deleteRoom(int id);
    int getNextRoomId() const;
    
    // Admin Diagnosis operations
    void deleteDiagnosis(int id);
    
    // Admin login
    bool adminLogin(int id, const QString& password) const;
    // Admin helpers
    QList<Admin> getAllAdmins() const;
    Admin getAdminById(int id) const;
    Admin getAdminByEmail(const QString& email) const;
    bool adminLoginByEmail(const QString& email, const QString& password) const;
    void updateAdmin(const Admin& admin); // CHANGED: Add updateAdmin method
    
    // Authentication
    bool patientLoginByEmail(const QString& email, const QString& password) const;
    bool doctorLoginByEmail(const QString& email, const QString& password) const;
    bool managerLoginByEmail(const QString& email, const QString& password) const;
    Patient getPatientByEmail(const QString& email) const;
    Doctor getDoctorByEmail(const QString& email) const;
    Manager getManagerByEmail(const QString& email) const;

    // Invitation codes operations
    QString generateInvitationCode(int parentId);
    QList<InvitationCode> getInvitationCodes(int parentId) const;
    InvitationCode getInvitationCodeByCode(const QString& code) const;
    void useInvitationCode(const QString& code, int invitedUserId);
    int getNextInvitationCodeId() const;

private:
    QString dataPath;
    
    QJsonArray loadJson(const QString& filename) const;
    void saveJson(const QString& filename, const QJsonArray& data);
};

#endif // DATAMANAGER_H
