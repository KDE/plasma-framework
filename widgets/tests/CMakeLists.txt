include_directories(${CMAKE_CURRENT_SOURCE_DIR})
########### testLayouts ###############

set(testlayouts_SRCS
    testLayouts.cpp
)

kde4_add_executable(testLayouts ${testlayouts_SRCS} )
target_link_libraries( testLayouts ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} plasma)

