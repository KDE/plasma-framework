include_directories( ${CMAKE_SOURCE_DIR}/workspace/plasma/lib   )


########### next target ###############

set(testAppletInfo_SRCS testAppletInfo.cpp )

kde4_automoc(${testAppletInfo_SRCS})

kde4_add_executable(testAppletInfo ${testAppletInfo_SRCS})

target_link_libraries(testAppletInfo  ${KDE4_KDECORE_LIBS} )

