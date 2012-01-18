project(localebindings)

set(localebindings_SRCS
    locale.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${localebindings_SRCS})


add_library(localebindingsplugin SHARED ${localebindings_SRCS})
#FIXME
#Should i remove KDE4_PLASMA_LIBS?
#Should i put something?
target_link_libraries(localebindingsplugin ${KDE4_PLASMA_LIBS} ${QT_QTDECLARATIVE_LIBRARY} kdeclarative)

install(TARGETS localebindingsplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/locale)
install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/plasma/locale)
