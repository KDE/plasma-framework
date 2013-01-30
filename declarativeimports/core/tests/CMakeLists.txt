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

set(runnermodeltest_SRCS
    main.cpp
    dynamictreemodel.cpp
    modeltest.cpp
    ../runnermodel.cpp
    )

qt4_automoc(${runnermodeltest_SRCS})
#kde4_add_library(datamodelsplugin SHARED ${datamodels_SRCS})
kde4_add_executable(runnermodeltest ${runnermodeltest_SRCS})
target_link_libraries(runnermodeltest ${QT_QTTEST_LIBRARY} ${KDE4_PLASMA_LIBS})
