
find_package(KF5 MODULE REQUIRED COMPONENTS InstallDirs)


# plasma_install_package(path componentname)
#
# Installs a Plasma package to the system path
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
#
macro(plasma_install_package dir component)
   set(root ${ARGV2})
   set(type ${ARGV3})
   if(NOT root)
      set(root plasma/plasmoids)
   endif()
   if(NOT type)
      set(type applet)
   endif()
   install(DIRECTORY ${dir}/ DESTINATION ${DATA_INSTALL_DIR}/${root}/${component} PATTERN .svn EXCLUDE PATTERN CMakeLists.txt EXCLUDE PATTERN Messages.sh EXCLUDE)
   install(FILES ${dir}/metadata.desktop DESTINATION ${SERVICES_INSTALL_DIR} RENAME plasma-${type}-${component}.desktop)
endmacro()


# plasma_add_plugin(pluginname sources_SRC)
#
# Use instead of add_library. Replacement for kde4_add_plugin
# Basically does add_library and removes the prefix of the library
#
# @arg pluginname The name of the plugin,
# @arg componentname The source files to be built
#
macro(plasma_add_plugin plugin)
    set(plugin_sources ${ARGN} )
    add_library(${plugin} MODULE ${plugin_sources} )
    set_target_properties(${plugin} PROPERTIES PREFIX "")
endmacro()

