add_library(plasma_dataengine_example_sourcesOnRequest MODULE sourcesOnRequest.cpp)

target_link_libraries(plasma_dataengine_example_sourcesOnRequest
  Qt${QT_MAJOR_VERSION}::Gui
  KF5::Plasma
  KF5::Service
)

install(TARGETS plasma_dataengine_example_sourcesOnRequest DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
