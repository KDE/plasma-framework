include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR})

set(qtjolie_LIB_SRCS
  abstractadaptor.cpp
  client.cpp
  clientthread.cpp
  value.cpp
  fault.cpp
  message.cpp
  metaservice.cpp
  pendingcall.cpp
  pendingcallwatcher.cpp
  pendingreply.cpp
  server.cpp
  serverthread.cpp
)

kde4_add_library(QtJoliePlasmaPrivate SHARED ${qtjolie_LIB_SRCS})

target_link_libraries(QtJoliePlasmaPrivate ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS QtJoliePlasmaPrivate
        DESTINATION ${LIB_INSTALL_DIR})

set_target_properties(QtJoliePlasmaPrivate PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(FILES
        abstractadaptor.h
        client.h
        value.h
        fault.h
        message.h
        metaservice.h
        pendingcall.h
        pendingcallwatcher.h
        pendingreply.h
        server.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/qtjolie)

if(NOT WIN32) # pkgconfig file
   configure_file(${CMAKE_CURRENT_SOURCE_DIR}/QtJolie.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/QtJoliePlasmaPrivate.pc @ONLY)
   install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QtJoliePlasmaPrivate.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)
endif(NOT WIN32)

configure_file("QtJolie.prf.cmake" "${CMAKE_BINARY_DIR}/QtJoliePlasmaPrivate.prf" @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/QtJoliePlasmaPrivate.prf" DESTINATION ${CMAKE_INSTALL_PREFIX}/mkspecs/features)
