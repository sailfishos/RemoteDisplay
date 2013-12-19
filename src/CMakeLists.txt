project(RemoteDisplay)
cmake_minimum_required(VERSION 2.8)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-std=gnu++0x")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_AUTOMOC TRUE)
find_package(Qt4 REQUIRED QtCore QtGui)
find_package(WinPR)
find_package(FreeRDP)

include(${QT_USE_FILE})
include_directories(${FreeRDP_INCLUDE_DIR} ${WinPR_INCLUDE_DIR})

aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST global.h remotedisplaywidget_p.h freerdphelpers.h)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS REMOTEDISPLAY_LIBRARY)
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} freerdp-core freerdp-codec freerdp-cache)
