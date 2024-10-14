cmake_minimum_required(VERSION 3.10)
project(myproj VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)

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

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/assets)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm glad)