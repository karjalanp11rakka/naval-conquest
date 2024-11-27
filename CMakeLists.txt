cmake_minimum_required(VERSION 3.10)
project(myproj VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "Specify the build type (Release or Debug) by setting CMAKE_BUILD_TYPE.")
endif()

option(USE_TOOLS "Run the Python scripts in 'tools' before building the project" OFF)
if(USE_TOOLS)
    message(STATUS "Running python scripts in 'tools'")
    find_program(PYTHON_EXECUTABLE NAMES python3 python)
    if(PYTHON_EXECUTABLE)
        file(GLOB_RECURSE PYTHON_TOOLS "tools/*.py")
        foreach(script IN LISTS PYTHON_TOOLS)
            execute_process(COMMAND ${PYTHON_EXECUTABLE} ${script})
        endforeach()
    else()
        message(WARNING "Python not found on the system")
    endif()
endif()

file(GLOB_RECURSE SRC_FILES "src/*.c" "src/*.cpp")

# GLAD
add_library(glad lib/glad/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/lib)

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glfw glm)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    remove_definitions(NDEBUG)
endif()

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm glad)