find_package(KF5Parts ${KF_DEP_VERSION} REQUIRED)
set_package_properties(KF5Parts PROPERTIES PURPOSE "Required for examples")

find_package(KF5WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
set_package_properties(KF5WidgetsAddons PROPERTIES PURPOSE "Required for examples")

add_subdirectory(applets)
add_subdirectory(containments)
add_subdirectory(wallpapers)
add_subdirectory(dataengines)
add_subdirectory(testcontainmentactionsplugin)
add_subdirectory(developerguide)
add_subdirectory(shell)
