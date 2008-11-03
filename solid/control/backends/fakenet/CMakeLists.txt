
add_subdirectory( tests )

########### next target ###############

set(solid_fakenet_PART_SRCS
   fakeaccesspoint.cpp
   fakenetworkinterface.cpp
   fakewirednetworkinterface.cpp
   fakewirelessnetworkinterface.cpp
   fakenetworkmanager.cpp
)


kde4_add_library(solid_fakenet_static STATIC ${solid_fakenet_PART_SRCS} )
target_link_libraries(solid_fakenet_static solidcontrol solidcontrolifaces ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} )

kde4_add_plugin(solid_fakenet module.cpp ${solid_fakenet_PART_SRCS})


target_link_libraries(solid_fakenet solidcontrol solidcontrolifaces ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} )

install(TARGETS solid_fakenet DESTINATION ${PLUGIN_INSTALL_DIR}  )


########### install files ###############

install( FILES solid_fakenet.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends  )
install( FILES fakenetworking.xml DESTINATION ${DATA_INSTALL_DIR}/solidfakenetbackend  )


