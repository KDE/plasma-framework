add_library(plasma_dataengine_example_customDataContainers MODULE)

target_sources(plasma_dataengine_example_customDataContainers PRIVATE
    customDataContainersEngine.cpp
    httpContainer.cpp
)

target_link_libraries(plasma_dataengine_example_customDataContainers
  KF5::Plasma
  KF5::KIOCore
  KF5::Service
)

install(TARGETS plasma_dataengine_example_customDataContainers DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
