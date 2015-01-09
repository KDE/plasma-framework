add_library(plasma_engine_testengine MODULE testengine.cpp)

kcoreaddons_desktop_to_json(plasma_engine_testengine plasma-dataengine-testengine.desktop)


target_link_libraries(plasma_engine_testengine
                      KF5::KIOWidgets KF5::Plasma)

install(TARGETS plasma_engine_testengine DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-testengine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
