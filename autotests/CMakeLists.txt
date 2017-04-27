find_package(Qt5Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt5Test PROPERTIES PURPOSE "Required for tests")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(KF5CoreAddons REQUIRED)
find_package(KF5XmlGui REQUIRED)
find_package(Qt5Widgets REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KIO TRUE)
    set(PLASMA_NO_PACKAGEKIT TRUE)
    set(PLASMA_NO_PACKAGE_EXTRADATA TRUE)
endif()

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               add_executable(${_testname} ${_testname}.cpp)
               target_include_directories(${_testname} PRIVATE "$<BUILD_INTERFACE:$<TARGET_PROPERTY:KF5PlasmaQuick,INCLUDE_DIRECTORIES>>;")
               target_link_libraries(${_testname} Qt5::Qml Qt5::Test
                                     KF5::Plasma KF5::PlasmaQuick
                                     KF5::Archive KF5::CoreAddons
                                     KF5::ConfigGui
                                     KF5::I18n
                                     KF5::KIOCore
                                     KF5::Service
                                     KF5::IconThemes
                                     KF5::Declarative)
               if(QT_QTOPENGL_FOUND)
                   target_link_libraries(${_testname} Qt5::OpenGL)
               endif()
               add_test(plasma-${_testname} ${_testname})
               ecm_mark_as_test(${_testname})
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    dialogqmltest
    dialogstatetest
    fallbackpackagetest
    packagestructuretest
    packageurlinterceptortest
    pluginloadertest
    framesvgtest
    iconitemtest
    themetest
    configmodeltest
    #    plasmoidpackagetest
)

add_executable(storagetest storagetest.cpp ../src/plasma/private/storage.cpp ../src/plasma/private/storagethread.cpp ../src/plasma/debug_p.cpp)
target_link_libraries(storagetest Qt5::Gui Qt5::Test Qt5::Sql KF5::KIOCore KF5::Plasma KF5::CoreAddons)

if(HAVE_X11)
    set(dialognativetest_srcs dialognativetest.cpp)
    ecm_add_test(${dialognativetest_srcs} TEST_NAME dialognativetest LINK_LIBRARIES Qt5::Gui Qt5::Test Qt5::Qml Qt5::Quick KF5::WindowSystem KF5::Plasma KF5::PlasmaQuick)
endif()

set(coronatest_srcs coronatest.cpp)
qt5_add_resources(coronatest_srcs coronatestresources.qrc)
ecm_add_test(${coronatest_srcs} TEST_NAME coronatest LINK_LIBRARIES Qt5::Gui Qt5::Widgets Qt5::Test KF5::KIOCore KF5::Plasma KF5::CoreAddons KF5::XmlGui)

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


#Add a test that i18n is not used directly in any import.
# It should /always/ be i18nd
find_program(SH bash)
add_test(i18ndcheck ${SH} ${CMAKE_CURRENT_SOURCE_DIR}/i18ndcheck.sh ${CMAKE_SOURCE_DIR}/src/declarativeimports)
