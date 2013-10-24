project(qtextracomponents)

add_subdirectory(tests)

set(qtextracomponents_SRCS
    qtextracomponentsplugin.cpp
    qpixmapitem.cpp
    qimageitem.cpp
    qiconitem.cpp
    mouseeventlistener.cpp
    columnproxymodel.cpp
    )

add_library(qtextracomponentsplugin SHARED ${qtextracomponents_SRCS})

target_link_libraries(qtextracomponentsplugin
        Qt5::Core
        ${Qt5Quick_LIBRARIES}
        ${Qt5Qml_LIBRARIES}
        Qt5::Gui
        KF5::KDeclarative
        KF5::KIconThemes
        Plasma)

install(TARGETS qtextracomponentsplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/qtextracomponents)

install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/qtextracomponents)
