project(draganddrop)

set(declarativedragdrop_SRCS
    draganddropplugin.cpp
    DeclarativeDragArea.cpp
    DeclarativeDragDropEvent.cpp
    DeclarativeDropArea.cpp
    DeclarativeMimeData.cpp
    )

add_library(draganddropplugin SHARED ${declarativedragdrop_SRCS})
target_link_libraries(draganddropplugin
        Qt5::Core
        Qt5::Quick
        Qt5::Qml
        Qt5::Gui
        Qt5::Widgets
)

install(TARGETS draganddropplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/draganddrop)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/draganddrop)
