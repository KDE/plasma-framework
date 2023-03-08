find_package(KF6Parts ${KF_DEP_VERSION} REQUIRED)
set_package_properties(KF6Parts PROPERTIES PURPOSE "Required for examples")

find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
set_package_properties(KF6WidgetsAddons PROPERTIES PURPOSE "Required for examples")

add_subdirectory(applets)
add_subdirectory(containments)
add_subdirectory(wallpapers)
add_subdirectory(testcontainmentactionsplugin)
add_subdirectory(developerguide)
add_subdirectory(shell)
