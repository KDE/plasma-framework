find_package(Qt${QT_MAJOR_VERSION}Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt${QT_MAJOR_VERSION}Test PROPERTIES PURPOSE "Required for tests")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(KF5CoreAddons REQUIRED)
find_package(KF5XmlGui REQUIRED)
find_package(Qt${QT_MAJOR_VERSION}Widgets REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_PACKAGE_EXTRADATA TRUE)
endif()

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               set(libs Qt${QT_MAJOR_VERSION}::Qml Qt${QT_MAJOR_VERSION}::Test KF5::Plasma KF5::PlasmaQuick
                        KF5::Archive KF5::CoreAddons KF5::ConfigGui KF5::I18n
                        KF5::Service KF5::IconThemes
                        KF5::Declarative)
               if(QT_QTOPENGL_FOUND)
                   list(APPEND libs Qt${QT_MAJOR_VERSION}::OpenGL)
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
        LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Qml Qt${QT_MAJOR_VERSION}::Quick KF5::WindowSystem KF5::Plasma KF5::PlasmaQuick
    )
    ecm_add_test(
        iconitemhidpitest.cpp
        TEST_NAME iconitemhidpitest
        LINK_LIBRARIES Qt::Gui Qt::Test Qt::Qml Qt::Quick KF5::IconThemes KF5::WindowSystem KF5::Plasma KF5::PlasmaQuick
    )
    set_tests_properties(iconitemhidpitest PROPERTIES ENVIRONMENT "QT_SCALE_FACTOR=1.25")
endif()

ecm_add_test(
    coronatest.cpp
    coronatestresources.qrc
    TEST_NAME coronatest
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::Test KF5::Plasma KF5::CoreAddons KF5::XmlGui
)

ecm_add_test(
    sortfiltermodeltest.cpp
    ../src/declarativeimports/core/datamodel.cpp
    ../src/declarativeimports/core/datasource.cpp
    TEST_NAME plasma-sortfiltermodeltest
    LINK_LIBRARIES KF5::Plasma Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Test KF5::I18n KF5::Service Qt${QT_MAJOR_VERSION}::Qml
)


#Add a test that i18n is not used directly in any import.
# It should /always/ be i18nd
find_program(SH bash)
if(SH)
    add_test(i18ndcheck ${SH} ${CMAKE_CURRENT_SOURCE_DIR}/i18ndcheck.sh ${CMAKE_SOURCE_DIR}/src/declarativeimports)
endif()
