project(calendar)
include(KDE4Defaults)

set(calendar_SRCS
    calendarplugin.cpp
    #incidencemodifier.cpp
    calendar.cpp
    calendardata.cpp
    #calendardayhelper.cpp
    #calendarroleproxymodel.cpp
    #datetimerangefiltermodel.cpp
    daysmodel.cpp
)

qt4_automoc(${calendar_SRCS})

add_library(calendarplugin SHARED ${calendar_SRCS})

target_link_libraries(calendarplugin
      Qt5::Core
    ${Qt5Quick_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS}
)

install(TARGETS calendarplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/calendar)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/calendar)

