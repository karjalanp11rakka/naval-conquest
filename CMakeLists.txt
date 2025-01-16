cmake_minimum_required(VERSION 3.12)
project(naval-conquest VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT WIN32)# assets_converter.py is not supported on Windows
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
endif()

file(GLOB_RECURSE SRC_FILES "src/*.cpp" "lib/glad.c")

include(FetchContent)

# GLFW
option(FETCH_GLFW "Fetch GLFW and build it into the project" ON) #not recommended for Linux
if(FETCH_GLFW)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)
else()
    find_package(glfw3 REQUIRED)
endif()
# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    remove_definitions(NDEBUG)
endif()

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm)

if(CMAKE_INSTALL_PREFIX)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()