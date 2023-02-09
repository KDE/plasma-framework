find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
set_package_properties(Qt6Test PROPERTIES PURPOSE "Required for tests")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})
remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(KF6CoreAddons REQUIRED)
find_package(KF6XmlGui REQUIRED)
find_package(Qt6Widgets REQUIRED)

# add_definitions( -DKDESRCDIR=${CMAKE_CURRENT_SOURCE_DIR} )

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_PACKAGE_EXTRADATA TRUE)
endif()

MACRO(PLASMA_UNIT_TESTS)
       FOREACH(_testname ${ARGN})
               set(libs Qt6::Qml Qt6::Test KF6::Plasma KF6::PlasmaQuick
                        KF6::Archive KF6::CoreAddons KF6::ConfigGui KF6::I18n
                        KF6::Service KF6::IconThemes
                        KF6::Declarative)
               if(QT_QTOPENGL_FOUND)
                   list(APPEND libs Qt6::OpenGL)
               endif()
               ecm_add_test(${_testname}.cpp
                            LINK_LIBRARIES ${libs}
                            NAME_PREFIX "plasma-")
               target_include_directories(${_testname} PRIVATE "$<BUILD_INTERFACE:$<TARGET_PROPERTY:KF6PlasmaQuick,INCLUDE_DIRECTORIES>>;")
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

if(HAVE_X11)
    ecm_add_test(
        dialognativetest.cpp
        TEST_NAME dialognativetest
        LINK_LIBRARIES Qt6::Gui Qt6::Test Qt6::Qml Qt6::Quick KF6::WindowSystem KF6::Plasma KF6::PlasmaQuick
    )
    ecm_add_test(
        iconitemhidpitest.cpp
        TEST_NAME iconitemhidpitest
        LINK_LIBRARIES Qt::Gui Qt::Test Qt::Qml Qt::Quick KF6::IconThemes KF6::WindowSystem KF6::Plasma KF6::PlasmaQuick
    )
    set_tests_properties(iconitemhidpitest PROPERTIES ENVIRONMENT "QT_SCALE_FACTOR=1.25")
endif()

ecm_add_test(
    coronatest.cpp
    coronatestresources.qrc
    TEST_NAME coronatest
    LINK_LIBRARIES Qt6::Gui Qt6::Widgets Qt6::Test KF6::Plasma KF6::CoreAddons KF6::XmlGui
)

ecm_add_test(
    sortfiltermodeltest.cpp
    ../src/declarativeimports/core/datamodel.cpp
    ../src/declarativeimports/core/datasource.cpp
    TEST_NAME plasma-sortfiltermodeltest
    LINK_LIBRARIES KF6::Plasma Qt6::Gui Qt6::Test KF6::I18n KF6::Service Qt6::Qml
)


#Add a test that i18n is not used directly in any import.
# It should /always/ be i18nd
find_program(SH bash)
if(SH)
    add_test(i18ndcheck ${SH} ${CMAKE_CURRENT_SOURCE_DIR}/i18ndcheck.sh ${CMAKE_SOURCE_DIR}/src/declarativeimports)
endif()
