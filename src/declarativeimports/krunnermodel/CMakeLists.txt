project(runnermodel)

set(runnermodel_SRCS
    ../core/runnermodel.cpp
    runnermodelplugin.cpp
    )

qt4_automoc(${runnermodel_SRCS})

add_library(runnermodelplugin SHARED ${runnermodel_SRCS})
target_link_libraries(runnermodelplugin
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
        plasma)


install(TARGETS runnermodelplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/runnermodel)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/runnermodel)

