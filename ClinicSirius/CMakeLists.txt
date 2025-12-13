cmake_minimum_required(VERSION 3.20)

project(ClinicSirius LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

find_package(Qt6 COMPONENTS Core Gui Widgets QUIET)
if(NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
  set(QT_VERSION_MAJOR 5)
else()
  set(QT_VERSION_MAJOR 6)
endif()

# Include directories
include_directories(include include/common include/doctors include/patients include/managers include/admins)

# Source files - organized by modules
set(COMMON_SOURCES
  src/common/main.cpp
  src/common/mainwindow.cpp
  src/common/authwindow.cpp
  src/common/loginwindow.cpp
  src/common/registrationwindow.cpp
  src/common/mainpage.cpp
  src/common/datamanager.cpp
  src/common/navigationwidget.cpp
  src/common/contentpage.cpp
  src/common/infocard.cpp
  src/common/styledbuttongroup.cpp
  src/common/dashboardwidget.cpp
  src/common/scrollabletab.cpp
  src/common/detailmenu.cpp
  src/common/statsbadge.cpp
)

# set(DOCTORS_SOURCES
#   src/doctors/doctorwidget.cpp
# )

set(DOCTORS_SOURCES
  src/doctors/doctorwidget.cpp
  src/doctors/doctorvisitdialog.cpp
  src/doctors/addslotdialog.cpp
  src/doctors/doctorprofilewidget.cpp
  src/doctors/patienthistorywidget.cpp
)

set(PATIENTS_SOURCES
  src/patients/profilewidget.cpp
  src/patients/createpatientdialog.cpp
  src/patients/addbycodedialog.cpp
  src/patients/appointmentbookingwidget.cpp
  src/patients/appointmentcalendar.cpp
)

set(MANAGERS_SOURCES
  src/managers/managerwidget.cpp
  src/managers/managerprofilewidget.cpp
  src/managers/managerscheduleviewer.cpp
  src/managers/patientmanagementdialog.cpp
  src/managers/bulkoperationsdialog.cpp
)

# set(ADMINS_SOURCES
#   src/admins/adminwidget.cpp
# )

# Header files - organized by modules
set(COMMON_HEADERS
  include/common/mainwindow.h
  include/common/authwindow.h
  include/common/loginwindow.h
  include/common/registrationwindow.h
  include/common/mainpage.h
  include/common/datamanager.h
  include/common/models.h
  include/common/navigationwidget.h
  include/common/contentpage.h
  include/common/infocard.h
  include/common/styledbuttongroup.h
  include/common/dashboardwidget.h
  include/common/scrollabletab.h
  include/common/detailmenu.h
  include/common/statsbadge.h
)

set(DOCTORS_HEADERS
  include/doctors/doctorwidget.h
  include/doctors/doctorvisitdialog.h
  include/doctors/addslotdialog.h
  include/doctors/doctorprofilewidget.h
  include/doctors/patienthistorywidget.h
)
set(PATIENTS_HEADERS
  include/patients/profilewidget.h
  include/patients/createpatientdialog.h
  include/patients/addbycodedialog.h
  include/patients/appointmentbookingwidget.h
  include/patients/appointmentcalendar.h
)

set(MANAGERS_HEADERS
  include/managers/managerwidget.h
  include/managers/managerprofilewidget.h
  include/managers/managerscheduleviewer.h
  include/managers/patientmanagementdialog.h
  include/managers/bulkoperationsdialog.h
)

# set(ADMINS_HEADERS
#   include/admins/adminwidget.h
# )

set(SOURCES
  ${COMMON_SOURCES}
  ${DOCTORS_SOURCES}
  ${PATIENTS_SOURCES}
  ${MANAGERS_SOURCES}
  # ${ADMINS_SOURCES}
)

set(HEADERS
  ${COMMON_HEADERS}
  ${DOCTORS_HEADERS}
  ${PATIENTS_HEADERS}
  ${MANAGERS_HEADERS}
  # ${ADMINS_HEADERS}
)

add_executable(ClinicSirius
  ${SOURCES}
  ${HEADERS}
  resources/resources.qrc
)

target_link_libraries(ClinicSirius PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets)


add_custom_command(TARGET ClinicSirius POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/../data
    ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data directory to build directory"
)

include(GNUInstallDirs)
install(TARGETS ClinicSirius
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

include(GNUInstallDirs)
install(TARGETS ClinicSirius
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
