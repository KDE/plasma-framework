add_library(plasma_dataengine_example_simpleEngine MODULE simpleEngine.cpp)

target_link_libraries(plasma_dataengine_example_simpleEngine
  Qt${QT_MAJOR_VERSION}::Gui
  KF5::Plasma
  KF5::Service
  KF5::I18n
)

install(TARGETS plasma_dataengine_example_simpleEngine DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
