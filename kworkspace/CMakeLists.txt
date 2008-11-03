

set(kworkspace_LIB_SRCS kdisplaymanager.cpp
                        kworkspace.cpp
                        kwindowlistmenu.cpp
   )

set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface( kworkspace_LIB_SRCS ${ksmserver_xml} ksmserver_interface )

set(kwin_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/kwin/org.kde.KWin.xml)

set_source_files_properties(${kwin_xml} PROPERTIES INCLUDE "interface_util.h")

qt4_add_dbus_interface( kworkspace_LIB_SRCS ${kwin_xml} kwin_interface )


kde4_add_library(kworkspace SHARED ${kworkspace_LIB_SRCS})

target_link_libraries(kworkspace  ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES} )

set_target_properties(kworkspace PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kworkspace ${INSTALL_TARGETS_DEFAULT_ARGS} )

install( FILES kdisplaymanager.h
               kworkspace.h
               kwindowlistmenu.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/kworkspace COMPONENT Devel )
