project(corebindings)

set(corebindings_SRCS
    corebindingsplugin.cpp
    dataengineconsumer.cpp
    theme.cpp
    datasource.cpp
    svgitem.cpp
    framesvgitem.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${corebindings_SRCS})


add_library(corebindingsplugin SHARED ${corebindings_SRCS})
target_link_libraries(corebindingsplugin ${KDE4_PLASMA_LIBS} ${QT_QTSCRIPT_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})

install(TARGETS corebindingsplugin DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/plasma/core)
install(FILES qmldir DESTINATION ${PLUGIN_INSTALL_DIR}/imports/org/kde/plasma/core)
