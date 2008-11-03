project(nepomukqueryclient)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_INCLUDE_DIR}
  ${nepomukquery_SOURCE_DIR}
  )

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})# -fPIC)

set(nepomukqueryclient_SRC
  queryserviceclient.cpp
)

set_source_files_properties(
  org.kde.nepomuk.QueryService.xml
  PROPERTIES INCLUDE "querymetatype.h")

set_source_files_properties(
  org.kde.nepomuk.Query.xml
  PROPERTIES INCLUDE "result.h")

qt4_add_dbus_interface(nepomukqueryclient_SRC
  org.kde.nepomuk.QueryService.xml
  queryserviceinterface)
qt4_add_dbus_interface(nepomukqueryclient_SRC
  org.kde.nepomuk.Query.xml
  queryinterface)

kde4_add_library(nepomukqueryclient SHARED ${nepomukqueryclient_SRC})

set_target_properties(nepomukqueryclient PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(nepomukqueryclient
  nepomukquery
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${SOPRANO_LIBRARIES}
  )

install(TARGETS nepomukqueryclient ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  nepomukqueryclient_export.h
  queryserviceclient.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk
)
