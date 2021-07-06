find_package(Qt5Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt5Test PROPERTIES PURPOSE "Required for tests")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(KF5CoreAddons REQUIRED)
find_package(KF5XmlGui REQUIRED)
find_package(Qt5Widgets REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_PACKAGE_EXTRADATA TRUE)
endif()

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               set(libs Qt5::Qml Qt5::Test KF5::Plasma KF5::PlasmaQuick
                        KF5::Archive KF5::CoreAddons KF5::ConfigGui KF5::I18n
                        KF5::Service KF5::IconThemes
                        KF5::Declarative)
               if(QT_QTOPENGL_FOUND)
                   list(APPEND libs Qt5::OpenGL)
               endif()
               ecm_add_test(${_testname}.cpp
                            LINK_LIBRARIES ${libs}
                            NAME_PREFIX "plasma-")
               target_include_directories(${_testname} PRIVATE "$<BUILD_INTERFACE:$<TARGET_PROPERTY:KF5PlasmaQuick,INCLUDE_DIRECTORIES>>;")
       ENDFOREACH(_testname)
ENDMACRO(PLASMA_UNIT_TESTS)

PLASMA_UNIT_TESTS(
    dialogqmltest
    dialogstatetest
    pluginloadertest
    framesvgtest
    iconitemtest
    themetest
)

if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
   EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.83.0)
    PLASMA_UNIT_TESTS(configmodeltest)
endif()

if(HAVE_X11)
    ecm_add_test(
        dialognativetest.cpp
        TEST_NAME dialognativetest
        LINK_LIBRARIES Qt5::Gui Qt5::Test Qt5::Qml Qt5::Quick KF5::WindowSystem KF5::Plasma KF5::PlasmaQuick
    )
endif()

ecm_add_test(
    coronatest.cpp
    coronatestresources.qrc
    TEST_NAME coronatest
    LINK_LIBRARIES Qt5::Gui Qt5::Widgets Qt5::Test KF5::Plasma KF5::CoreAddons KF5::XmlGui
)

ecm_add_test(
    sortfiltermodeltest.cpp
    ../src/declarativeimports/core/datamodel.cpp
    ../src/declarativeimports/core/datasource.cpp
    TEST_NAME plasma-sortfiltermodeltest
    LINK_LIBRARIES KF5::Plasma Qt5::Gui Qt5::Test KF5::I18n KF5::Service Qt5::Qml
)


#Add a test that i18n is not used directly in any import.
# It should /always/ be i18nd
find_program(SH bash)
if(SH)
    add_test(i18ndcheck ${SH} ${CMAKE_CURRENT_SOURCE_DIR}/i18ndcheck.sh ${CMAKE_SOURCE_DIR}/src/declarativeimports)
endif()
