project(RemoteDisplay)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_AUTOMOC TRUE)
find_package(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})

aux_source_directory(. SRC_LIST)
include(global.h)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS REMOTEDISPLAY_LIBRARY)
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES})
