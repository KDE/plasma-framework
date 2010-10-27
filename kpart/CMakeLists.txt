
set(plasma_kpartscripting_SRCS
   scripting/layouttemplatepackagestructure.cpp
   scripting/rect.cpp
   scripting/scriptengine.cpp
   scripting/widget.cpp
)

set(plasma_kpart_SRCS
    ${plasma_kpartscripting_SRCS}
    plasmakpartcorona.cpp
    plasmakpartview.cpp
    plasmakpart.cpp
)

kde4_add_plugin(plasma-kpart ${plasma_kpart_SRCS})

target_link_libraries(plasma-kpart ${QT_QTSCRIPT_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${X11_LIBRARIES} ${KDE4_KFILE_LIBS} ${KDE4_KPARTS_LIBS} )

install(TARGETS plasma-kpart DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-kpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES plasmakpart.h plasmakpartview.h plasmakpartcorona.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )
