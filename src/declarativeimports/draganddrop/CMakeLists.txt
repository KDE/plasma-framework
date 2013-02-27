project(draganddrop)

include(KDE4Defaults)

set(declarativedragdrop_SRCS
    draganddropplugin.cpp
    DeclarativeDragArea.cpp
    DeclarativeDragDropEvent.cpp
    DeclarativeDropArea.cpp
    DeclarativeMimeData.cpp
    )

INCLUDE_DIRECTORIES(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${KDE4_INCLUDES}
)

qt4_automoc(${declarativedragdrop_SRCS})


add_library(draganddropplugin SHARED ${declarativedragdrop_SRCS})
target_link_libraries(draganddropplugin
        ${QT_QTCORE_LIBRARY}
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
)

install(TARGETS draganddropplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/draganddrop)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/draganddrop)
