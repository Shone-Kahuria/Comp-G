cmake_minimum_required(VERSION 3.24)

project(ComputerGraphicsExperiments6And7 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMake 4.x no longer implicitly enables compatibility policies used by
# freeglut 3.6.0's build scripts.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

# Build freeglut with the project so Visual Studio users do not have to copy
# headers, DLLs, or library files by hand.
set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(FREEGLUT_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(INSTALL_PDB OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    freeglut
    GIT_REPOSITORY https://github.com/freeglut/freeglut.git
    GIT_TAG v3.6.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(freeglut)

find_package(OpenGL REQUIRED)

if(TARGET freeglut_static)
    set(FREEGLUT_TARGET freeglut_static)
elseif(TARGET freeglut)
    set(FREEGLUT_TARGET freeglut)
else()
    message(FATAL_ERROR "The downloaded freeglut project did not define a usable target.")
endif()

function(add_graphics_experiment target source)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE ${freeglut_SOURCE_DIR}/include)
    target_compile_definitions(${target} PRIVATE FREEGLUT_STATIC)
    target_link_libraries(${target} PRIVATE ${FREEGLUT_TARGET} OpenGL::GL OpenGL::GLU)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_graphics_experiment(experiment6_sierpinski src/experiment6_sierpinski.cpp)
add_graphics_experiment(experiment7_bezier_flag src/experiment7_bezier_flag.cpp)

