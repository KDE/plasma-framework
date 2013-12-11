set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)

find_package(KCoreAddons REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname}
                                     Qt5::Test
                                     Qt5::Declarative
                                     KF5::Plasma
                                     KF5::KArchive KF5::KCoreAddons
                                     KF5::KConfigGui
                                     KF5::KI18n
                                     KF5::KIOCore
                                     KF5::KService
                                     KF5::PlasmaQuick
                                    )
               if(QT_QTOPENGL_FOUND)
                   target_link_libraries(${_testname} Qt5::OpenGL)
               endif()
               add_test(plasma-${_testname} ${_testname})
               ecm_mark_as_test(${_testname})
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    packageurlinterceptortest
)


