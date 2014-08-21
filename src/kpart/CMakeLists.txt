
set(plasma_kpartscripting_SRCS
   scripting/layouttemplatepackagestructure.cpp
   scripting/rect.cpp
   scripting/scriptengine.cpp
   scripting/widget.cpp
)

set(plasma_kpart_SRCS
   # ${plasma_kpartscripting_SRCS}
    plasmakpartcorona.cpp
    plasmakpartview.cpp
    plasmakpart.cpp
)

add_library(plasma-kpart MODULE ${plasma_kpart_SRCS})

target_link_libraries(plasma-kpart
        Qt5::Script
        KF5::Plasma
        KF5::PlasmaQuick
        KF5::KIOCore
        KF5::KIOWidgets
        ${X11_LIBRARIES}
        KF5::Parts
        Qt5::Quick)

install(TARGETS plasma-kpart DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-kpart.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(test)
