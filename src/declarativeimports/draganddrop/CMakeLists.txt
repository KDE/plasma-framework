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


kde4_add_library(draganddropplugin SHARED ${declarativedragdrop_SRCS})
target_link_libraries(draganddropplugin ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY})

install(TARGETS draganddropplugin DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/draganddrop)

install(FILES qmldir DESTINATION ${IMPORTS_INSTALL_DIR}/org/kde/draganddrop)
