project(localebindings)

set(localebindings_SRCS
    locale.cpp
    localebindingsplugin.cpp
    calendarsystem.cpp
)

find_package(KDE4Support REQUIRED)

add_library(localebindingsplugin SHARED ${localebindings_SRCS})
target_link_libraries(localebindingsplugin
        Qt5::Core
        Qt5::Quick
        Qt5::Qml
        KF5::KDE4Support
        KF5::KI18n
        )
install(TARGETS localebindingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/locale)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/locale)
