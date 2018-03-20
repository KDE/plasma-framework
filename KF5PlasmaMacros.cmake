
find_package(ECM 1.6.0 CONFIG REQUIRED)
include(${ECM_KDE_MODULE_DIR}/KDEInstallDirs.cmake)

set(PLASMA_RELATIVE_DATA_INSTALL_DIR "plasma")
set(PLASMA_DATA_INSTALL_DIR "${KDE_INSTALL_DATADIR}/${PLASMA_RELATIVE_DATA_INSTALL_DIR}")

# plasma_install_package(path componentname [root] [type])
#
# Use plasma_install_bundled_package instead.
# Installs a Plasma package to the system path
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
# @arg root The subdirectory to install to, default: plasmoids
# @arg type The type, default to applet, or applet, package, containment,
#       wallpaper, shell, lookandfeel, etc.
# @see Types column in kpackagetool5 --list-types
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

   kpackage_install_package(${dir} ${component} ${root} ${PLASMA_RELATIVE_DATA_INSTALL_DIR} NO_DEPRECATED_WARNING)

   install(FILES ${dir}/metadata.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME plasma-${type}-${component}.desktop)
endmacro()


# plasma_install_bundled_package(path componentname [root] [type])
#
# Installs a Plasma package to the system path,
# compressing all its files in a binary rcc qresources file.
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
# @arg root The subdirectory to install to, default: plasmoids
# @arg type The type, default to applet, or applet, package, containment,
#       wallpaper, shell, lookandfeel, etc.
# @see Types column in kpackagetool5 --list-types
#
# Examples:
# plasma_install_bundled_package(mywidget org.kde.plasma.mywidget) # installs an applet
# plasma_install_bundled_package(declarativetoolbox org.kde.toolbox packages package) # installs a generic package
#
macro(plasma_install_bundled_package dir component)
   set(root ${ARGV2})
   set(type ${ARGV3})
   if(NOT root)
      set(root plasmoids)
   endif()
   if(NOT type)
      set(type applet)
   endif()

   kpackage_install_bundled_package(${dir} ${component} ${root} ${PLASMA_RELATIVE_DATA_INSTALL_DIR})

   install(FILES ${dir}/metadata.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} RENAME plasma-${type}-${component}.desktop)
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

