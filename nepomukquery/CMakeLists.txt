project(nepomukquery)

include_directories(
  ${QT_INCLUDES}
  ${KDE4_INCLUDES}
  ${CMAKE_SOURCE_DIR}
  ${SOPRANO_INCLUDE_DIR}
  ${NEPOMUK_INCLUDE_DIR}
  )

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})# -fPIC)

set(nepomukquery_SRC
  result.cpp
  term.cpp
  query.cpp
  queryparser.cpp
  dbusoperators.cpp
)

kde4_add_library(nepomukquery SHARED ${nepomukquery_SRC}
)

set_target_properties(nepomukquery PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(nepomukquery
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${SOPRANO_LIBRARIES}
  )

install(TARGETS nepomukquery ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  nepomukquery_export.h
  queryparser.h
  result.h
  term.h
  query.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/nepomuk
)
