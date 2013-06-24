set(
   kded_platformstatus_SRCS
   platformstatus.cpp
   splash/splashwindow.cpp
   ../shell/lookandfeelpackage.cpp
   )

include_directories("../utils")

qt4_add_dbus_adaptor (
   kded_platformstatus_SRCS
   org.kde.platformstatus.xml
   platformstatus.h PlatformStatus
   )

kde4_add_plugin( kded_platformstatus ${kded_platformstatus_SRCS} )

target_link_libraries(
   kded_platformstatus
   ${KDE4_KDECORE_LIBS}
   ${KCoreAddons_LIBRARIES}
   plasma
   Qt5::DBus
   Qt5::Quick
   )

install( TARGETS kded_platformstatus        DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kded_platformstatus.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES org.kde.platformstatus.xml   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

