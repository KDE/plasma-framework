

########### next target ###############

set(solidcontrolifaces_LIB_SRCS 
   powermanager.cpp
   networkinterface.cpp
   networkcdmainterface.cpp
   networkgsminterface.cpp
   networkserialinterface.cpp
   networkmanager.cpp
   wirednetworkinterface.cpp
   wirelessaccesspoint.cpp
   wirelessnetworkinterface.cpp
   bluetoothremotedevice.cpp
   bluetoothinputdevice.cpp
   bluetoothinterface.cpp
   bluetoothmanager.cpp
   bluetoothsecurity.cpp )

set(unused
   networkmanager.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp

   )

kde4_add_library(solidcontrolifaces SHARED ${solidcontrolifaces_LIB_SRCS})

target_link_libraries(solidcontrolifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(solidcontrolifaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrolifaces ${INSTALL_TARGETS_DEFAULT_ARGS}  )


########### install files ###############

install( FILES powermanager.h networkmanager.h networkinterface.h networkcdmainterface.h networkgsminterface.h networkserialinterface.h wirednetworkinterface.h wirelessnetworkinterface.h wirelessaccesspoint.h bluetoothremotedevice.h bluetoothinputdevice.h bluetoothinterface.h bluetoothmanager.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid/control/ifaces COMPONENT Devel)

