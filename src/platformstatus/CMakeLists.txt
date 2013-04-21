set(kded_platformstatus_SRCS  platformstatus.cpp )

kde4_add_plugin(kded_platformstatus ${kded_platformstatus_SRCS})

target_link_libraries(kded_platformstatus ${KDE4_KDECORE_LIBS} ${KCoreAddons_LIBRARIES} Qt5::DBus)

install(TARGETS kded_platformstatus DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kded-platformstatus.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES org.kde.platformstatus.xml  DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )

