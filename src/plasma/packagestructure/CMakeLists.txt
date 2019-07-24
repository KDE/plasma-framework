function(install_package_structure name)
    add_library(${name}_packagestructure MODULE ${name}package.cpp packages.cpp)
    target_link_libraries(${name}_packagestructure PRIVATE KF5::Package KF5::Plasma KF5::Declarative KF5::I18n)
    install(TARGETS ${name}_packagestructure DESTINATION ${KDE_INSTALL_PLUGINDIR}/kpackage/packagestructure)

    set_target_properties(
        ${name}_packagestructure
        PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/kpackage/packagestructure"
    )

endfunction()

install_package_structure(plasmageneric)
install_package_structure(plasmoid)
install_package_structure(dataengine)
install_package_structure(plasmatheme)
install_package_structure(containmentactions)

