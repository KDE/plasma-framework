set(QML_INSTALL_DIR "${LIB_INSTALL_DIR}/qml")
message("!!! Forcing Imports install location: ${QML_INSTALL_DIR}")
message("!!! Remove once merged into cmake-extra-modules")


add_subdirectory(core)
#add_subdirectory(draganddrop)
#add_subdirectory(krunnermodel)
#add_subdirectory(qtextracomponents)
add_subdirectory(plasmacomponents)
#add_subdirectory(plasmaextracomponents)
#add_subdirectory(locale)
