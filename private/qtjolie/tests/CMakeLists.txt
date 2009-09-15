include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../includes ${CMAKE_CURRENT_SOURCE_DIR}/../)

MACRO(SODEP_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp)
    target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} QtJolie)
    add_definitions(-DDATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
  ENDFOREACH(_testname)
ENDMACRO(SODEP_UNIT_TESTS)

MACRO(SODEP_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
    target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} QtJolie)
    add_definitions(-DDATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
  ENDFOREACH(_testname)
ENDMACRO(SODEP_EXECUTABLE_TESTS)

SODEP_UNIT_TESTS(
  testvalue
  testfault
  testmessage
  testmetaservice
)

SODEP_EXECUTABLE_TESTS(
  testprinter
  calculatorservice
  trivialyahooclient
)

set(visiondriver_SRCS
   visiondriver.cpp
)
kde4_add_ui_files(visiondriver_SRCS visiondriverwidget.ui)

kde4_add_executable(visiondriver ${visiondriver_SRCS})
target_link_libraries(visiondriver ${KDE4_KDEUI_LIBS} ${QT_QTNETWORK_LIBRARY} QtJolie)

