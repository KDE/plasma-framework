add_executable(plasmoidloader
    main.cpp
    appletviewer.cpp
)

#widgets linked for QApplication. QtQuickControls behaves differently depending on if it's linked or not
target_link_libraries(plasmoidloader KF5::Plasma KF5::PlasmaQuick Qt5::Widgets     Qt5::Script)

#install(TARGETS plugintest ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

