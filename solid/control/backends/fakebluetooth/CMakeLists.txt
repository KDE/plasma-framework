
add_subdirectory( tests )

########### next target ###############

set(solid_fakebluetooth_PART_SRCS
   fakebluetoothinterface.cpp
   fakebluetoothmanager.cpp
   fakebluetoothremotedevice.cpp
   fakebluetoothinputdevice.cpp
   fakebluetoothsecurity.cpp
)


kde4_add_library(solid_fakebluetooth_static STATIC ${solid_fakebluetooth_PART_SRCS} )
target_link_libraries(solid_fakebluetooth_static solidcontrolifaces ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} )

kde4_add_plugin(solid_fakebluetooth module.cpp ${solid_fakebluetooth_PART_SRCS})


target_link_libraries(solid_fakebluetooth solidcontrolifaces ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} )

install(TARGETS solid_fakebluetooth DESTINATION ${PLUGIN_INSTALL_DIR}  )


########### install files ###############

install( FILES solid_fakebluetooth.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends  )
install( FILES fakebluetooth.xml DESTINATION ${DATA_INSTALL_DIR}/solidfakebluetoothbackend  )


