


########### next target ###############

set(taskmanager_LIB_SRCS
   abstractgroupableitem.cpp
   abstractgroupingstrategy.cpp
   abstractsortingstrategy.cpp
   groupmanager.cpp
   startup.cpp
   strategies/alphasortingstrategy.cpp
   strategies/programgroupingstrategy.cpp
   strategies/manualgroupingstrategy.cpp
   strategies/manualsortingstrategy.cpp
   task.cpp
   taskactions.cpp
   taskgroup.cpp
   taskitem.cpp
   taskmanager.cpp
   taskrmbmenu.cpp
   )

kde4_add_library(taskmanager SHARED ${taskmanager_LIB_SRCS})

target_link_libraries(taskmanager ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})
if (X11_Xfixes_FOUND)
  target_link_libraries(taskmanager ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)
if (X11_Xrender_FOUND)
  target_link_libraries(taskmanager ${X11_Xrender_LIB})
endif (X11_Xrender_FOUND)
if (X11_Xcomposite_FOUND)
  target_link_libraries(taskmanager ${X11_Xcomposite_LIB})
endif (X11_Xcomposite_FOUND)

set_target_properties(taskmanager PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS taskmanager ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES
   abstractgroupableitem.h
   abstractgroupingstrategy.h
   abstractsortingstrategy.h
   groupmanager.h
   startup.h
   task.h
   taskactions.h
   taskgroup.h
   taskitem.h
   taskmanager.h
   taskmanager_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/taskmanager COMPONENT Devel
   )


