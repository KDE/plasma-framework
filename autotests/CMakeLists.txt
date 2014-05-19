find_package(Qt5Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt5Test PROPERTIES PURPOSE "Required for tests")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)

find_package(KF5CoreAddons REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               target_link_libraries(${_testname} Qt5::Qml Qt5::Test
                                     KF5::Plasma KF5::PlasmaQuick
                                     KF5::Archive KF5::CoreAddons
                                     KF5::ConfigGui
                                     KF5::I18n
                                     KF5::KIOCore
                                     KF5::Service)
               if(QT_QTOPENGL_FOUND)
                   target_link_libraries(${_testname} Qt5::OpenGL)
               endif()
               add_test(plasma-${_testname} ${_testname})
               ecm_mark_as_test(${_testname})
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    packagestructuretest
    packageurlinterceptortest
    pluginloadertest
    #    plasmoidpackagetest
)

add_executable(storagetest storagetest.cpp ../src/plasma/private/storage.cpp ../src/plasma/private/storagethread.cpp)
target_link_libraries(storagetest Qt5::Gui Qt5::Test Qt5::Sql KF5::KIOCore KF5::Plasma KF5::CoreAddons )
add_test(plasma-storagetest storagetest)
ecm_mark_as_test(storagetest)

add_executable(sortfiltermodeltest
    sortfiltermodeltest.cpp
    ../src/declarativeimports/core/datamodel.cpp
    ../src/declarativeimports/core/datasource.cpp
    )

target_link_libraries(sortfiltermodeltest
    KF5::Plasma
    Qt5::Gui
    Qt5::Test
    KF5::I18n
    KF5::Service
    Qt5::Qml
    )

add_test(plasma-sortfiltermodeltest sortfiltermodeltest)
ecm_mark_as_test(sortfiltermodeltest)

if(QT_QTOPENGL_FOUND)
  target_link_libraries(storagetest Qt5::OpenGL)
endif()
