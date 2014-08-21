set(plasmaparttest_SRCS
   main.cpp
   plasmaparttest.cpp
 )

add_executable(plasmaparttest ${plasmaparttest_SRCS})

target_link_libraries(plasmaparttest
    KF5::Parts
)

