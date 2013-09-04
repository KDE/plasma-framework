set(sourcesOnRequest_SRCS
    sourcesOnRequest.cpp
)

kde4_add_plugin(plasma_dataengine_example_sourcesOnRequest ${sourcesOnRequest_SRCS})
target_link_libraries(plasma_dataengine_example_sourcesOnRequest ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_dataengine_example_sourcesOnRequest DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-example-sourcesOnRequest.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
