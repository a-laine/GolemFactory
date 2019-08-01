cmake_minimum_required (VERSION 3.13)
project(GolemFactory LANGUAGES CXX)

#######################
# external dependencies
#######################

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)

include_directories(${GLEW_INCLUDE_DIR})
include_directories(${glfw3_INCLUDE_DIR})
include_directories(${ASSIMP_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIR})

#######################
# set variables
#######################

# global build options
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)


#######################
# configure projects
#######################

# configure project directories
file(GLOB_RECURSE SOURCE_FILE_LIST Sources/*)
include_directories(Sources/)

# declare project
add_executable(${PROJECT_NAME} ${SOURCE_FILE_LIST})

# declare dependencies
target_link_libraries(${PROJECT_NAME} ${OPENGL_gl_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})

