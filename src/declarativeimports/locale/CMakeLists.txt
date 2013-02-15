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

qt4_automoc(${localebindings_SRCS})

add_library(localebindingsplugin SHARED ${localebindings_SRCS})
target_link_libraries(localebindingsplugin
        ${QT_QTCORE_LIBRARY}
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
        ${KDE4_KDECORE_LIBS}
        )
install(TARGETS localebindingsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/locale)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/locale)
