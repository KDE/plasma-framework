project(localebindings)

include(KDE4Defaults)

set(localebindings_SRCS
    locale.cpp
    localebindingsplugin.cpp
    calendarsystem.cpp
)

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${localebindings_SRCS})


kde4_add_library(localebindingsplugin SHARED ${localebindings_SRCS})

target_link_libraries(localebindingsplugin ${QT_QTDECLARATIVE_LIBRARY} ${KDE4_KDECORE_LIBRARY})

install(TARGETS localebindingsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/locale)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/locale)
