project(sodep)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QT_INCLUDE_DIR})

set(sodep_LIB_SRCS sodepclient.cpp sodepclientthread.cpp sodepvalue.cpp sodepfault.cpp sodepmessage.cpp)

kde4_add_library(sodep SHARED ${sodep_LIB_SRCS})

target_link_libraries(sodep ${QT_QTCORE_LIBRARY})

install(TARGETS sodep
        DESTINATION ${LIB_INSTALL_DIR})

set_target_properties(sodep PROPERTIES VERSION 1.0.0 SOVERSION 1)

install(FILES
        sodepclient.h
        sodepvalue.h
        sodepfault.h
        sodepmessage.h
        DESTINATION ${INCLUDE_INSTALL_DIR})
