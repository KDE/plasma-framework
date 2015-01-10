set(
   kded_platformstatus_SRCS
   platformstatus.cpp
   )

qt5_add_dbus_adaptor (
   kded_platformstatus_SRCS
   org.kde.platformstatus.xml
   platformstatus.h PlatformStatus
   )

add_library(kded_platformstatus MODULE ${kded_platformstatus_SRCS} )
set_target_properties(kded_platformstatus PROPERTIES
    OUTPUT_NAME platformstatus
)

target_link_libraries(
   kded_platformstatus
   KF5::Service
   KF5::CoreAddons
   KF5::DBusAddons
   KF5::I18n
   KF5::KIOCore KF5::KIOWidgets
   Qt5::DBus
   Qt5::Quick
)

install( TARGETS kded_platformstatus        DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded )
install( FILES platformstatus.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kded )
install( FILES org.kde.platformstatus.xml   DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )

