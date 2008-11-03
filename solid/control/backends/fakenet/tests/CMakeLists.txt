set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(fakenetworktest_SRCS fakenetworktest.cpp )


kde4_add_unit_test(fakenetworktest ${fakenetworktest_SRCS})

target_link_libraries(fakenetworktest solid_fakenet_static solidcontrolifaces ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )
add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../fakenetworking.xml\\"")

########### install files ###############



