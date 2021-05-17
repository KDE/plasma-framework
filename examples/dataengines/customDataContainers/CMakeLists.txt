add_library(plasma_dataengine_example_customDataContainers MODULE)

target_sources(plasma_dataengine_example_customDataContainers PRIVATE
    customDataContainersEngine.cpp
    httpContainer.cpp
)

kcoreaddons_desktop_to_json(plasma_dataengine_example_customDataContainers plasma-dataengine-example-customDataContainers.desktop)

target_link_libraries(plasma_dataengine_example_customDataContainers
  KF5::Plasma
  KF5::KIOCore
  KF5::Service
)

install(TARGETS plasma_dataengine_example_customDataContainers DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES plasma-dataengine-example-customDataContainers.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
