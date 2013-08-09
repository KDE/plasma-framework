
find_package(KF5 MODULE REQUIRED COMPONENTS InstallDirs)

macro(installPackage dir component)
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

macro(plasma_add_plugin plugin)
    set(plugin_sources ${ARGN} )
    add_library(${plugin} MODULE ${plugin_sources} )
    set_target_properties(${plugin} PROPERTIES PREFIX "")
endmacro()

