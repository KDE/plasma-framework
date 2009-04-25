project(QtJolie)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR})

set(qtjolie_LIB_SRCS sodepclient.cpp sodepclientthread.cpp sodepvalue.cpp sodepfault.cpp sodepmessage.cpp sodeppendingcall.cpp)

kde4_add_library(QtJolie SHARED ${qtjolie_LIB_SRCS})

target_link_libraries(QtJolie ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS QtJolie
        DESTINATION ${LIB_INSTALL_DIR})

set_target_properties(QtJolie PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(FILES
        sodepclient.h
        sodepvalue.h
        sodepfault.h
        sodepmessage.h
        sodeppendingcall.h
        DESTINATION ${INCLUDE_INSTALL_DIR})
