if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
    add_definitions(-DHAVE_XCB_COMPOSITE=1)
    include_directories(
        ${XCB_XCB_INCLUDE_DIR}
        ${XCB_DAMAGE_INCLUDE_DIR}
        ${XCB_COMPOSITE_INCLUDE_DIR}
    )
else()
    add_definitions(-DHAVE_XCB_COMPOSITE=0)
endif()
if(HAVE_X11 AND XCB_XCB_FOUND AND XCB_SHAPE_FOUND)
    add_definitions(-DHAVE_XCB_SHAPE=1)
    include_directories(${XCB_SHAPE_INCLUDE_DIR})
else()
    add_definitions(-DHAVE_XCB_SHAPE=0)
endif()

set(plasmaquick_LIB_SRC
    appletquickitem.cpp
    dialog.cpp
    dialogshadows.cpp
    view.cpp
    configmodel.cpp
    shellpluginloader.cpp
    configview.cpp
    packageurlinterceptor.cpp
    private/configcategory_p.cpp
    private/packages.cpp
    ../declarativeimports/core/framesvgitem.cpp
    ../declarativeimports/core/units.cpp
)

add_library(KF5PlasmaQuick SHARED ${plasmaquick_LIB_SRC})
add_library(KF5::PlasmaQuick ALIAS KF5PlasmaQuick)
target_include_directories(KF5PlasmaQuick PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")

target_link_libraries(KF5PlasmaQuick
    PUBLIC
        Qt5::Quick
        Qt5::Qml
        KF5::Plasma
    PRIVATE
        KF5::WindowSystem
        KF5::KIOWidgets
        KF5::I18n
        KF5::IconThemes
        KF5::Service
        KF5::CoreAddons
        KF5::XmlGui
        KF5::Declarative
)

if(HAVE_X11)
    target_link_libraries(KF5PlasmaQuick
        PRIVATE
            Qt5::X11Extras
            ${X11_LIBRARIES}
            ${XCB_XCB_LIBRARY}
    )

    if(XCB_COMPOSITE_FOUND AND XCB_DAMAGE_FOUND)
        target_link_libraries(KF5PlasmaQuick
            PRIVATE
                ${XCB_DAMAGE_LIBRARY}
                ${XCB_COMPOSITE_LIBRARY}
        )
    endif()

    if(XCB_SHAPE_FOUND)
        target_link_libraries(KF5PlasmaQuick PRIVATE ${XCB_SHAPE_LIBRARY})
    endif()

    if(HAVE_GLX)
        target_link_libraries(KF5PlasmaQuick PRIVATE ${OPENGL_gl_LIBRARY})
    endif()
endif()

set_target_properties(KF5PlasmaQuick PROPERTIES
                                     VERSION ${PLASMA_VERSION_STRING}
                                     SOVERSION ${PLASMA_SOVERSION}
                                     EXPORT_NAME PlasmaQuick
                      )

install(TARGETS KF5PlasmaQuick EXPORT KF5PlasmaQuickTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

generate_export_header(KF5PlasmaQuick BASE_NAME PlasmaQuick)

set(plasmaquick_LIB_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/plasmaquick_export.h
    appletquickitem.h
    view.h
    configview.h
    configmodel.h
    packageurlinterceptor.h
)

#install(FILES ${plasmaquick_LIB_INCLUDES}
#        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/plasmaquick COMPONENT Devel)

#install(DIRECTORY
#        includes/PlasmaQuick
#        DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KDE COMPONENT Devel)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5PlasmaQuick")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF5PlasmaQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF5PlasmaQuickConfig.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}" COMPONENT Devel
)

install(EXPORT KF5PlasmaQuickTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5PlasmaQuickTargets.cmake NAMESPACE KF5:: )
