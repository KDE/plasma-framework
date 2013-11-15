project(runnermodel)

set(runnermodel_SRCS
    ../core/runnermodel.cpp
    runnermodelplugin.cpp
    )

add_library(runnermodelplugin SHARED ${runnermodel_SRCS})
target_link_libraries(runnermodelplugin
        Qt5::Quick
        Qt5::Qml
	KF5::KI18n
        Plasma
	)


install(TARGETS runnermodelplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/runnermodel)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/runnermodel)

