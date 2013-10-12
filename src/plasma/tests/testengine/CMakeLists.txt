project(plasma_testengine_dataengine)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

set(testengine_engine_SRCS
    testengine.cpp)

plasma_add_plugin(plasma_engine_testengine
                ${testengine_engine_SRCS})

target_link_libraries(plasma_engine_testengine
                      ${KDE4_KIO_LIBS}
                      ${Plasma_LIBRARIES})

install(TARGETS plasma_engine_testengine DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-testengine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
