set(apptemplate_DIRS
    qml-plasmoid
    qml-plasmoid-with-qml-extension
    cpp-plasmoid
    plasma-wallpaper
    plasma-wallpaper-with-qml-extension
)

kde_package_app_templates(TEMPLATES ${apptemplate_DIRS} INSTALL_DIR ${KDE_INSTALL_KTEMPLATESDIR})
