INCLUDE_DIRECTORIES(
    ${corebindings_SOURCE_DIR}
    )

kde4_add_unit_test(sortfiltermodeltest
    sortfiltermodeltest.cpp
    ${corebindings_SOURCE_DIR}/dataengineconsumer.cpp
    ${corebindings_SOURCE_DIR}/datamodel.cpp
    ${corebindings_SOURCE_DIR}/datasource.cpp
    )

qt4_automoc(sortfiltermodeltest.cpp)

target_link_libraries(sortfiltermodeltest
    ${KDE4_PLASMA_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    )
