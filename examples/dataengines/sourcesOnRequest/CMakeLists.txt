add_library(plasma_dataengine_example_sourcesOnRequest MODULE sourcesOnRequest.cpp)

kcoreaddons_desktop_to_json(plasma_dataengine_example_sourcesOnRequest plasma-dataengine-example-sourcesOnRequest.desktop)

target_link_libraries(plasma_dataengine_example_sourcesOnRequest
  Qt5::Gui
  KF5::Plasma
  KF5::Service
)

install(TARGETS plasma_dataengine_example_sourcesOnRequest DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES plasma-dataengine-example-sourcesOnRequest.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
