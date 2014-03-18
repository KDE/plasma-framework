INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/..")

include(ECMMarkAsTest)

add_executable(fullmodelaccesstest columnproxymodeltest.cpp ../columnproxymodel.cpp ../../core/tests/modeltest.cpp)
target_link_libraries(fullmodelaccesstest
    Qt5::Gui
    Qt5::Test
    )

add_test(fullmodelaccesstest fullmodelaccesstest)
ecm_mark_as_test(fullmodelaccesstest)
