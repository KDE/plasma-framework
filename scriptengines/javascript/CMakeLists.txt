# APPLET

set(simple_javascript_engine_SRCS
    simplejavascriptapplet.cpp
    appletinterface.cpp
    uiloader.cpp
    simplebindings/anchorlayout.cpp
    simplebindings/color.cpp
    simplebindings/font.cpp
    simplebindings/filedialogproxy.cpp
    simplebindings/graphicsitem.cpp
    simplebindings/linearlayout.cpp
    simplebindings/painter.cpp
    simplebindings/pixmap.cpp
    simplebindings/point.cpp
    simplebindings/rect.cpp
    simplebindings/size.cpp
    simplebindings/timer.cpp
    simplebindings/url.cpp
)

include_directories(${PHONON_INCLUDES})


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
install(FILES plasma-scriptengine-applet-simple-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

# RUNNER

set(javascript_runner_engine_SRCS
    javascriptrunner.cpp
)

kde4_add_plugin(plasma_runnerscript_javascript ${javascript_runner_engine_SRCS})

target_link_libraries(plasma_runnerscript_javascript
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_PLASMA_LIBS}
                       ${QT_QTSCRIPT_LIBRARY}
                       ${QT_QTUITOOLS_LIBRARY}
                       ${QT_QTXML_LIBRARY}
                       )

install(TARGETS plasma_runnerscript_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-scriptengine-runner-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR})


# DATAENGINE

set(javascript_dataengine_engine_SRCS
    javascriptdataengine.cpp
)
kde4_add_plugin(plasma_dataenginescript_javascript ${javascript_dataengine_engine_SRCS})

target_link_libraries(plasma_dataenginescript_javascript
                       ${KDE4_KDECORE_LIBS}
                       ${KDE4_PLASMA_LIBS}
                       ${QT_QTSCRIPT_LIBRARY})

install(TARGETS plasma_dataenginescript_javascript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-scriptengine-dataengine-javascript.desktop DESTINATION ${SERVICES_INSTALL_DIR})

