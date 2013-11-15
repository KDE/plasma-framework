# APPLET

if(KDE_PLATFORM_FEATURE_BINARY_COMPATIBLE_FEATURE_REDUCTION)
    set(PLASMA_NO_KDEWEBKIT TRUE)
    set(PLASMA_NO_SOLID TRUE)
    set(PLASMA_NO_KIO TRUE)
endif()

set(simple_javascript_engine_SRCS
    common/extension_launchapp.cpp
    common/extension_io.cpp
    common/guiscriptenv.cpp
    common/javascriptaddonpackagestructure.cpp
    declarative/toolboxproxy.cpp
    declarative/appletcontainer.cpp
    plasmoid/abstractjsappletscript.cpp
    plasmoid/appletauthorization.cpp
    plasmoid/jsappletinterface.cpp
    plasmoid/simplejavascriptapplet.cpp
    plasmoid/themedsvg.cpp
    simplebindings/animationgroup.cpp
    simplebindings/anchorlayout.cpp
    simplebindings/dataenginereceiver.cpp
    simplebindings/bytearrayclass.cpp
    simplebindings/bytearrayprototype.cpp
    simplebindings/color.cpp
    simplebindings/dataengine.cpp
    simplebindings/easingcurve.cpp
    simplebindings/font.cpp
    simplebindings/filedialogproxy.cpp
    simplebindings/graphicsitem.cpp
    simplebindings/icon.cpp
    simplebindings/i18n.cpp
    simplebindings/linearlayout.cpp
    simplebindings/gridlayout.cpp
    simplebindings/painter.cpp
    simplebindings/pen.cpp
    simplebindings/pixmap.cpp
    simplebindings/point.cpp
    simplebindings/rect.cpp
    simplebindings/qscriptbookkeeping.cpp
    simplebindings/size.cpp
    simplebindings/sizepolicy.cpp
    simplebindings/timer.cpp
    simplebindings/uiloader.cpp
    simplebindings/url.cpp
)

include_directories(${PHONON_INCLUDES})

add_library(plasma_appletscript_simple_javascript MODULE ${simple_javascript_engine_SRCS})

target_link_libraries(plasma_appletscript_simple_javascript
                       KF5::KIOCore KF5::KIOWidgets
                       KF5::Plasma
                       Qt5::Declarative
                       Qt5::Xml
                       )

install(TARGETS plasma_appletscript_simple_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-applet-simple-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

# RUNNER

set(javascript_runner_engine_SRCS
    common/extension_launchapp.cpp
    common/extension_io.cpp
    common/javascriptaddonpackagestructure.cpp
    common/scriptenv.cpp
    runner/javascriptrunner.cpp
    simplebindings/i18n.cpp
)

add_library(plasma_runnerscript_javascript MODULE ${javascript_runner_engine_SRCS})

target_link_libraries(plasma_runnerscript_javascript
                      KF5::KCoreAddons
                      KF5::KIOWidgets
                      KF5::Plasma
                      Qt5::Script)

install(TARGETS plasma_runnerscript_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-runner-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR})


# DATAENGINE

set(javascript_dataengine_engine_SRCS
    common/extension_launchapp.cpp
    common/extension_io.cpp
    common/javascriptaddonpackagestructure.cpp
    common/scriptenv.cpp
    dataengine/javascriptdataengine.cpp
    dataengine/javascriptservice.cpp
    simplebindings/dataengine.cpp
    simplebindings/i18n.cpp
    simplebindings/qscriptnonguibookkeeping.cpp
)
add_library(plasma_dataenginescript_javascript MODULE ${javascript_dataengine_engine_SRCS})

target_link_libraries(plasma_dataenginescript_javascript
                      KF5::KCoreAddons
                      KF5::KIOWidgets
                      KF5::Plasma
                      Qt5::Script)

install(TARGETS plasma_dataenginescript_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-dataengine-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# ADDONS

set(javascript_addon_packagestructure_SRCS
    common/addonpackageplugin.cpp
    common/javascriptaddonpackagestructure.cpp
)

add_library(plasma_packagestructure_javascriptaddon MODULE ${javascript_addon_packagestructure_SRCS})
target_link_libraries(plasma_packagestructure_javascriptaddon KF5::Plasma)

install(TARGETS plasma_packagestructure_javascriptaddon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-packagestructure-javascript-addon.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/plasma-javascriptaddon.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
