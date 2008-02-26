set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               kde4_add_unit_test(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} plasma ${QT_QTTEST_LIBRARY})
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

kde4_add_unit_test(packagestructuretest packagestructuretest.cpp) # ../packages.cpp)
target_link_libraries(packagestructuretest plasma ${QT_QTTEST_LIBRARY})

PLASMA_UNIT_TESTS(
    packagemetadatatest
    plasmoidpackagetest
)

