include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(plasma_LIB_SRCS
   plasma.cpp
   applet.cpp
   interface.cpp
   abstractrunner.cpp
   svg.cpp
   theme.cpp
   dataengine.cpp
   datavisualization.cpp

   widgets/widget.cpp
   widgets/lineedit.cpp
   widgets/pushbutton.cpp
   widgets/checkbox.cpp
   )

kde4_automoc(${plasma_LIB_SRCS})

kde4_add_library(plasma SHARED ${plasma_LIB_SRCS})

target_link_libraries(plasma  ${KDE4_KIO_LIBS} )

set_target_properties(plasma PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS plasma  DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES
    applet.h
    plasma.h
    abstractrunner.h
    theme.h
    interface.h
    dataengine.h
    svg.h

    widgets/widget.h
    widgets/lineedit.h
    widgets/pushbutton.h
    widgets/checkbox.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/plasma )

install( FILES
    includes/Applet
    includes/Plasma
    includes/AbstractRunner
    includes/Theme
    includes/Interface
    includes/DataEngine
    includes/Svg
    DESTINATION ${INCLUDE_INSTALL_DIR}/Plasma )

