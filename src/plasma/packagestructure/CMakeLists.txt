function(install_package_structure name)
    kcoreaddons_add_plugin(${name} SOURCES ${name}_packagestructure.cpp INSTALL_NAMESPACE "kpackage/packagestructure")
    target_link_libraries(${name} plasma_packagestructure_static)
endfunction()

add_library(plasma_packagestructure_static STATIC packages.cpp)
target_link_libraries(plasma_packagestructure_static KF5::Package KF5::Plasma KF5::Declarative KF5::I18n)

install_package_structure(plasma_generic)
install_package_structure(plasma_applet)
install_package_structure(plasma_dataengine)
install_package_structure(plasma_theme)
install_package_structure(plasma_containmentactions)

