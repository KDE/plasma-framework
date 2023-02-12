add_library(plasma_dataengine_example_sourcesOnRequest MODULE sourcesOnRequest.cpp)

target_link_libraries(plasma_dataengine_example_sourcesOnRequest
  Qt6::Gui
  KF6::Plasma
)

install(TARGETS plasma_dataengine_example_sourcesOnRequest DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
