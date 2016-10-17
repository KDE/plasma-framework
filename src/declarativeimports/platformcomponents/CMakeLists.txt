set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(platformcomponents_SRCS
    platformextensionplugin.cpp
    application.cpp
    icondialog.cpp
    )

ecm_qt_declare_logging_category(platformcomponents_SRCS HEADER debug_p.h IDENTIFIER LOG_PLASMAPLATFORMCOMPONENTS CATEGORY_NAME org.kde.plasma.imports.platformcomponents)

add_library(platformcomponentsplugin SHARED ${platformcomponents_SRCS})

target_link_libraries(
    platformcomponentsplugin
    Qt5::Core
    Qt5::DBus
    Qt5::Qml
    KF5::WindowSystem
    KF5::IconThemes
)

install(TARGETS platformcomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/platformcomponents)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/platformcomponents)

