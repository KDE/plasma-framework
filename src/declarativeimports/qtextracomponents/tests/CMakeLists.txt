INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/..")

include(ECMMarkAsTest)

add_executable(fullmodelaccesstest columnproxymodeltest.cpp ../columnproxymodel.cpp ../../core/tests/modeltest.cpp)
target_link_libraries(fullmodelaccesstest
    Qt5::Gui
    Qt5::Test
    KF5::KCoreAddons
    ${KDE4Support_LIBRARIES}
    KF5::KI18n
    )

add_test(plasma-fullmodelaccesstest fullmodelaccesstest)
ecm_mark_as_test(fullmodelaccesstest)
