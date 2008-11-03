set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

########### next target ###############

set(fakebluetoothtest_SRCS fakebluetoothtest.cpp )


kde4_add_unit_test(fakebluetoothtest ${fakebluetoothtest_SRCS})

target_link_libraries(fakebluetoothtest solid_fakebluetooth_static solidcontrolifaces ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} )
add_definitions(-DTEST_DATA="\\"${CMAKE_CURRENT_SOURCE_DIR}/../fakebluetooth.xml\\"")


########### install files ###############



