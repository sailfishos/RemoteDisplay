project(RemoteDisplayExample)
cmake_minimum_required(VERSION 2.8)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-std=gnu++0x")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_AUTOMOC TRUE)
find_package(Qt4 REQUIRED QtCore QtGui)
include(${QT_USE_FILE})

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

target_link_libraries(${PROJECT_NAME} RemoteDisplay ${QT_LIBRARIES})
include_directories(${RemoteDisplay_SOURCE_DIR})

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
