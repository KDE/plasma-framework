add_subdirectory(desktoptheme)
add_subdirectory(plasma)
add_subdirectory(declarativeimports)
add_subdirectory(plasmapkg)
add_subdirectory(scriptengines)
add_subdirectory(plasmaquick)

ecm_qt_install_logging_categories(
    EXPORT PLASMA
    FILE plasma-framework.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
