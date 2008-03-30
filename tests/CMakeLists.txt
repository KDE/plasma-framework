set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} plasma ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    packagestructuretest
    packagemetadatatest
    plasmoidpackagetest
)

set(appletbrowser_SRCS appletbrowser.cpp)
kde4_add_executable(plasmaappletbrowser ${appletbrowser_SRCS})
target_link_libraries(plasmaappletbrowser ${KDE4_KDEUI_LIBS} plasma)

set(sharedtimertest_SRCS sharedtimertest.cpp)
kde4_add_executable(sharedtimertest ${sharedtimertest_SRCS})
target_link_libraries(sharedtimertest ${KDE4_KDEUI_LIBS} plasma)

