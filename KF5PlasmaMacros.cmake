
find_package(ECM 0.0.9 CONFIG REQUIRED)
include(KDEInstallDirs)

set(PLASMA_RELATIVE_DATA_INSTALL_DIR "plasma")
set(PLASMA_DATA_INSTALL_DIR "${DATA_INSTALL_DIR}/${PLASMA_RELATIVE_DATA_INSTALL_DIR}")

# plasma_install_package(path componentname [root] [type])
#
# Installs a Plasma package to the system path
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
# @arg root The subdirectory to install to, default: plasmoids
# @arg type The type, default to applet, or applet, package, containment,
#       wallpaper, shell, lookandfeel, etc.
# @see Types column in plasmapkg --list-types
#
# Examples:
# plasma_install_package(mywidget org.kde.plasma.mywidget) # installs an applet
# plasma_install_package(declarativetoolbox org.kde.toolbox packages package) # installs a generic package
#
macro(plasma_install_package dir component)
   set(root ${ARGV2})
   set(type ${ARGV3})
   if(NOT root)
      set(root plasmoids)
   endif()
   if(NOT type)
      set(type applet)
   endif()
   install(DIRECTORY ${dir}/ DESTINATION ${PLASMA_DATA_INSTALL_DIR}/${root}/${component}
           PATTERN .svn EXCLUDE
           PATTERN CMakeLists.txt EXCLUDE
           PATTERN Messages.sh EXCLUDE
           PATTERN dummydata EXCLUDE)

   install(FILES ${dir}/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-${type}-${component}.desktop)
endmacro()


# plasma_add_plugin(pluginname sources_SRC)
#
# Use instead of add_library. Replacement for kde4_add_plugin
# Basically does add_library and removes the prefix of the library
#
# @arg pluginname The name of the plugin,
# @arg sources_SRC The source files to be built
#
# Example:
# plasma_add_plugin(plasma_engine_statusnotifieritem ${statusnotifieritem_engine_SRCS})
#
macro(plasma_add_plugin plugin)
    message(WARNING "plasma_add_plugin() is deprecated, use add_library(MODULE) instead. You can use the porting scripts in plasma-framework/tools")
    set(plugin_sources ${ARGN} )
    add_library(${plugin} MODULE ${plugin_sources} )
    set_target_properties(${plugin} PROPERTIES PREFIX "")
endmacro()

