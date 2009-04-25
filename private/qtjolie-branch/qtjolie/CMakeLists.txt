project(QtJolie)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/../
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR})

set(qtjolie_LIB_SRCS client.cpp clientthread.cpp value.cpp fault.cpp message.cpp pendingcall.cpp)

kde4_add_library(QtJolie SHARED ${qtjolie_LIB_SRCS})

target_link_libraries(QtJolie ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS QtJolie
        DESTINATION ${LIB_INSTALL_DIR})

set_target_properties(QtJolie PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(FILES
        client.h
        value.h
        fault.h
        message.h
        pendingcall.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/qtjolie)
