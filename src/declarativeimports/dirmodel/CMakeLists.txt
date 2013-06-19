project(dirmodel)

set(dirmodel_SRCS
    dirmodel.cpp
    dirmodelplugin.cpp
    )


add_library(dirmodelplugin SHARED ${dirmodel_SRCS})
target_link_libraries(dirmodelplugin
        ${QT_QTCORE_LIBRARY}
        ${Qt5Qml_LIBRARIES}
        ${KDE4_KIO_LIBS}
        KDE4__kde4support
        )

install(TARGETS dirmodelplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/dirmodel)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/dirmodel)

#add_subdirectory(test)
