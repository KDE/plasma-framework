# APPLET

set(simple_javascript_engine_SRCS
    common/javascriptaddonpackagestructure.cpp
    common/jsscriptenv.cpp
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

include_directories(${PHONON_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/common)

kde4_add_plugin(plasma_appletscript_simple_javascript ${simple_javascript_engine_SRCS})

target_link_libraries(plasma_appletscript_simple_javascript
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_KIO_LIBS}
                       ${KDE4_PLASMA_LIBS}
                       ${QT_QTSCRIPT_LIBRARY}
                       ${QT_QTUITOOLS_LIBRARY}
                       ${QT_QTXML_LIBRARY}
                       )

install(TARGETS plasma_appletscript_simple_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-applet-simple-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

# RUNNER

set(javascript_runner_engine_SRCS
    common/javascriptaddonpackagestructure.cpp
    common/scriptenv.cpp
    runner/javascriptrunner.cpp
    simplebindings/i18n.cpp
)

kde4_add_plugin(plasma_runnerscript_javascript ${javascript_runner_engine_SRCS})

target_link_libraries(plasma_runnerscript_javascript
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDE4_PLASMA_LIBS}
                      ${QT_QTSCRIPT_LIBRARY})

install(TARGETS plasma_runnerscript_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-runner-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR})


# DATAENGINE

set(javascript_dataengine_engine_SRCS
    common/javascriptaddonpackagestructure.cpp
    common/scriptenv.cpp
    dataengine/javascriptdataengine.cpp
    dataengine/javascriptservice.cpp
    simplebindings/dataengine.cpp
    simplebindings/i18n.cpp
    simplebindings/qscriptnonguibookkeeping.cpp
)
kde4_add_plugin(plasma_dataenginescript_javascript ${javascript_dataengine_engine_SRCS})

target_link_libraries(plasma_dataenginescript_javascript
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDE4_PLASMA_LIBS}
                      ${QT_QTSCRIPT_LIBRARY})

install(TARGETS plasma_dataenginescript_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-dataengine-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# ADDONS

set(javascript_addon_packagestructure_SRCS
    common/addonpackageplugin.cpp
    common/javascriptaddonpackagestructure.cpp
)

kde4_add_plugin(plasma_packagestructure_javascriptaddon ${javascript_addon_packagestructure_SRCS})
target_link_libraries(plasma_packagestructure_javascriptaddon ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_packagestructure_javascriptaddon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-packagestructure-javascript-addon.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/plasma-javascriptaddon.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

#DECLARATIVE APPLET
set(declarative_appletscript_SRCS
    common/javascriptaddonpackagestructure.cpp
    common/declarativescriptenv.cpp
    plasmoid/abstractjsappletscript.cpp
    plasmoid/appletauthorization.cpp
    plasmoid/appletinterface.cpp
    plasmoid/declarativeappletscript.cpp
    plasmoid/engineaccess.cpp
    plasmoid/themedsvg.cpp
    simplebindings/bytearrayclass.cpp
    simplebindings/bytearrayprototype.cpp
    simplebindings/dataengine.cpp
    simplebindings/dataenginereceiver.cpp
    simplebindings/filedialogproxy.cpp
    simplebindings/i18n.cpp
    simplebindings/icon.cpp
    simplebindings/qscriptbookkeeping.cpp
    simplebindings/url.cpp
    )

include_directories(${PHONON_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/common)

kde4_add_plugin(plasma_appletscript_declarative ${declarative_appletscript_SRCS})
target_link_libraries(plasma_appletscript_declarative
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTSCRIPT_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}

)


install(TARGETS plasma_appletscript_declarative DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-scriptengine-applet-declarative.desktop DESTINATION ${SERVICES_INSTALL_DIR})


