project(dirmodel)

set(dirmodel_SRCS
    dirmodel.cpp
    dirmodelplugin.cpp
    )


add_library(dirmodelplugin SHARED ${dirmodel_SRCS})
target_link_libraries(dirmodelplugin
        Qt5::Core
        Qt5::Qml
        KF5::KIOCore
        KF5::KIOWidgets
        KF5::KGuiAddons
        )

install(TARGETS dirmodelplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/dirmodel)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/dirmodel)

#add_subdirectory(test)
