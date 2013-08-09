INCLUDE_DIRECTORIES(
    ${corebindings_SOURCE_DIR}
    )

include(ECMMarkAsTest)

add_executable(sortfiltermodeltest
    sortfiltermodeltest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../plasma/dataengineconsumer.cpp
    ${corebindings_SOURCE_DIR}/datamodel.cpp
    ${corebindings_SOURCE_DIR}/datasource.cpp
    )

qt4_automoc(sortfiltermodeltest.cpp)

target_link_libraries(sortfiltermodeltest
    plasma
    ${KDE4Support_LIBRARIES}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${KI18n_LIBRARIES}
    )


add_test(plasma-sortfiltermodeltest sortfiltermodeltest)
ecm_mark_as_test(sortfiltermodeltest)

set(runnermodeltest_SRCS
    main.cpp
    dynamictreemodel.cpp
    modeltest.cpp
    ../runnermodel.cpp
    )

qt4_automoc(${runnermodeltest_SRCS})
add_executable(runnermodeltest ${runnermodeltest_SRCS})
target_link_libraries(runnermodeltest ${QT_QTTEST_LIBRARY} ${plasma_LIBRARIES} plasma)
ecm_mark_as_test(runnermodeltest)
