set(customDataContainers_SRCS
    customDataContainersEngine.cpp
    httpContainer.cpp
)

kservice_desktop_to_json(plasma-dataengine-example-customDataContainers.desktop)

add_library(plasma_dataengine_example_customDataContainers MODULE ${customDataContainers_SRCS})
target_link_libraries(plasma_dataengine_example_customDataContainers
  KF5::Plasma
  KF5::KIOCore
  KF5::Service
)

install(TARGETS plasma_dataengine_example_customDataContainers DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-customDataContainers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
