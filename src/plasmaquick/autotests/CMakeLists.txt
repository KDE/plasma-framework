set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)

find_package(KF5CoreAddons REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname}
                                     Qt5::Test
                                     Qt5::Qml
                                     KF5::Plasma
                                     KF5::Archive KF5::CoreAddons
                                     KF5::ConfigGui
                                     KF5::I18n
                                     KF5::KIOCore
                                     KF5::Service
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


