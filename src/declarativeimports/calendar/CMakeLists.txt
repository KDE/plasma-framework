project(calendar)

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

add_library(calendarplugin SHARED ${calendar_SRCS})

target_link_libraries(calendarplugin
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    Qt5::Gui
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KCALCORE_LIBS}
    ${KDEPIMLIBS_AKONADI_CALENDAR_LIBS}
)

install(TARGETS calendarplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/calendar)
install(DIRECTORY qml/ DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/calendar)

