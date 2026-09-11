cmake_minimum_required(VERSION 3.20)

project(ImplicitNet VERSION 0.1.0 LANGUAGES C CXX)

option(IMPLICIT_NET_BUILD_TESTS "Build the invariant test executable" ON)

add_library(implicit_net SHARED implicit_net.c)
add_library(ImplicitNet::implicit_net ALIAS implicit_net)

target_include_directories(implicit_net
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>)
target_compile_features(implicit_net PUBLIC c_std_17)
target_compile_definitions(implicit_net PRIVATE IMPLICIT_NET_BUILDING_LIBRARY)

if(MSVC)
    target_compile_options(implicit_net PRIVATE /W4 /permissive-)
else()
    target_compile_options(implicit_net PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(IMPLICIT_NET_BUILD_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_executable(implicit_net_c_test c_test.c)
        target_link_libraries(implicit_net_c_test PRIVATE implicit_net)
        target_compile_features(implicit_net_c_test PRIVATE c_std_17)
        if(MSVC)
            target_compile_options(implicit_net_c_test PRIVATE /W4 /permissive-)
        else()
            target_compile_options(implicit_net_c_test PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        add_test(NAME implicit_net.c.invariants COMMAND implicit_net_c_test)

        add_executable(implicit_net_cpp_test cpp_test.cpp)
        target_link_libraries(implicit_net_cpp_test PRIVATE implicit_net)
        target_compile_features(implicit_net_cpp_test PRIVATE cxx_std_11)
        if(MSVC)
            target_compile_options(implicit_net_cpp_test PRIVATE /W4 /permissive-)
        else()
            target_compile_options(implicit_net_cpp_test PRIVATE -Wall -Wextra -Wpedantic)
        endif()
        add_test(NAME implicit_net.cpp.linkage COMMAND implicit_net_cpp_test)

        find_package(Python3 COMPONENTS Interpreter QUIET)
        if(Python3_Interpreter_FOUND)
            add_test(
                NAME implicit_net.python.oracle
                COMMAND ${CMAKE_COMMAND} -E env
                    "IMPLICIT_NET_LIBRARY=$<TARGET_FILE:implicit_net>"
                    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_implicit_net.py)
        endif()
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS implicit_net
    EXPORT ImplicitNetTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES implicit_net.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
