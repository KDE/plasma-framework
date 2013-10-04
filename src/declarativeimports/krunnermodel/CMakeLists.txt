project(runnermodel)

set(runnermodel_SRCS
    ../core/runnermodel.cpp
    runnermodelplugin.cpp
    )

add_library(runnermodelplugin SHARED ${runnermodel_SRCS})
target_link_libraries(runnermodelplugin
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
	KF5::KI18n
        plasma
	)


install(TARGETS runnermodelplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/runnermodel)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/runnermodel)

