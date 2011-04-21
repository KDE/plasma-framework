project(components_test)

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )


set(components_test_TEST_SRCS
    test.cpp)

kde4_add_executable(components_test ${components_test_TEST_SRCS})

target_link_libraries(components_test kdeclarative ${KDE4_KDECORE_LIBS} ${QT_QTDECLARATIVE_LIBRARY})

## install


install(TARGETS components_test EXPORT kdelibsLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


install(FILES "gallery.qml" DESTINATION ${CMAKE_BINARY_DIR}/widgets/components)