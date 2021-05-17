add_library(plasma_dataengine_example_simpleEngine MODULE simpleEngine.cpp)

kcoreaddons_desktop_to_json(plasma_dataengine_example_simpleEngine plasma-dataengine-example-simpleEngine.desktop)

target_link_libraries(plasma_dataengine_example_simpleEngine
  Qt5::Gui
  KF5::Plasma
  KF5::Service
  KF5::I18n
)

install(TARGETS plasma_dataengine_example_simpleEngine DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES plasma-dataengine-example-simpleEngine.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
