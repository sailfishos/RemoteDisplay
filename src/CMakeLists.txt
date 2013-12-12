project(RemoteDisplay)
cmake_minimum_required(VERSION 2.8)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set(CMAKE_AUTOMOC TRUE)
find_package(Qt4 REQUIRED QtCore QtGui)
find_package(WinPR)
find_package(FreeRDP)

include(${QT_USE_FILE})
include_directories(${FreeRDP_INCLUDE_DIR} ${WinPR_INCLUDE_DIR})

aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST global.h)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS REMOTEDISPLAY_LIBRARY)
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} freerdp-core)
