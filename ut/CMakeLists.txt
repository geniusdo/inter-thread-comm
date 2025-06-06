
include_directories(${CMAKE_SOURCE_DIR})

file(GLOB TEST_SOURCES "*.cpp")


foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)

    add_executable(${test_name} ${test_src})

    # target_include_directories(${test_name}
    #     PRIVATE ${CMAKE_SOURCE_DIR}/src  
    # )
endforeach()