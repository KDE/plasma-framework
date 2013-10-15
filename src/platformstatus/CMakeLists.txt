set(
   kded_platformstatus_SRCS
   platformstatus.cpp
   )

qt4_add_dbus_adaptor (
   kded_platformstatus_SRCS
   org.kde.platformstatus.xml
   platformstatus.h PlatformStatus
   )

plasma_add_plugin(kded_platformstatus ${kded_platformstatus_SRCS} )

target_link_libraries(
   kded_platformstatus
   KF5::KService
   KF5::KCoreAddons
   KF5::KDBusAddons
   KF5::KI18n
   KF5::KIOCore KF5::KIOWidgets
   Qt5::DBus
   Qt5::Quick
   )

install( TARGETS kded_platformstatus        DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kded_platformstatus.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES org.kde.platformstatus.xml   DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

