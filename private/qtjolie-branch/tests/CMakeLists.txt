include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../sodep)

MACRO(SODEP_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp)
    target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTNETWORK_LIBRARY} sodep)
    add_definitions(-DDATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
  ENDFOREACH(_testname)
ENDMACRO(SODEP_UNIT_TESTS)

MACRO(SODEP_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
    target_link_libraries(${_testname} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} sodep)
    add_definitions(-DDATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}\\"")
  ENDFOREACH(_testname)
ENDMACRO(SODEP_EXECUTABLE_TESTS)

SODEP_UNIT_TESTS(
  sodepvaluetest
  sodepfaulttest
  sodepmessagetest
  sodepmetaservicetest
)

SODEP_EXECUTABLE_TESTS(
  sodepprintertest
)
