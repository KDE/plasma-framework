project(localebindings)

# include(KDE4Defaults)
#
set(localebindings_SRCS
    locale.cpp
    localebindingsplugin.cpp
    calendarsystem.cpp
)

# INCLUDE_DIRECTORIES(
#         ${CMAKE_SOURCE_DIR}
#         ${CMAKE_BINARY_DIR}
#         ${KDE4_INCLUDES}
# )

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
