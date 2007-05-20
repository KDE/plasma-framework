include_directories(${CMAKE_CURRENT_SOURCE_DIR})

########### next target ###############

set(plasma_LIB_SRCS
   abstractrunner.cpp
   applet.cpp
   dataengine.cpp
   datavisualization.cpp
   plasma.cpp
   interface.cpp
   svg.cpp
   theme.cpp
   widgets/checkbox.cpp
   widgets/lineedit.cpp
   widgets/pushbutton.cpp
   widgets/radiobutton.cpp
   widgets/widget.cpp
   )

kde4_automoc(${plasma_LIB_SRCS})

kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma  ${KDE4_KIO_LIBS} )

set_target_properties(plasma PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS plasma  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
    abstractrunner.h
    applet.h
    dataengine.h
    datavisualization.h
    interface.h
    plasma.h
    plasma_export.h
    svg.h
    theme.h
    widgets/widget.h
    widgets/lineedit.h
    widgets/pushbutton.h
    widgets/checkbox.h
    widgets/radiobutton.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma )

install( FILES
    includes/Applet
    includes/Plasma
    includes/AbstractRunner
    includes/Theme
    includes/Interface
    includes/DataEngine
    includes/Svg
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma )

install( FILES plasma_dataengine.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )

