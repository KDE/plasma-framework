set(customDataContainers_SRCS
    customDataContainersEngine.cpp
    httpContainer.cpp
)

kde4_add_plugin(plasma_dataengine_example_customDataContainers ${customDataContainers_SRCS})
target_link_libraries(plasma_dataengine_example_customDataContainers ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_dataengine_example_customDataContainers DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-customDataContainers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
