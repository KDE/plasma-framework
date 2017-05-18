set(%{APPNAMELC}plugin_SRCS
    %{APPNAMELC}plugin.cpp
)

add_library(%{APPNAMELC}plugin SHARED ${%{APPNAMELC}plugin_SRCS})

target_link_libraries(%{APPNAMELC}plugin
    Qt5::Gui
    Qt5::Qml
)
install(TARGETS %{APPNAMELC}plugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/%{APPNAMELC})
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/%{APPNAMELC})
