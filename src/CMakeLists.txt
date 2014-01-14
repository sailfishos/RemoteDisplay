project(RemoteDisplay)
cmake_minimum_required(VERSION 2.8)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-std=gnu++0x")
endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_AUTOMOC TRUE)
find_package(Qt4 OPTIONAL_COMPONENTS QtCore QtGui QtMultimedia)
find_package(WinPR)
find_package(FreeRDP)

if(QT_QTMULTIMEDIA_FOUND)
    set(WITH_QTSOUND 1)
endif(QT_QTMULTIMEDIA_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include(${QT_USE_FILE})
include_directories(${FreeRDP_INCLUDE_DIR} ${WinPR_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. SRC_LIST)
list(APPEND SRC_LIST
    global.h
    remotedisplaywidget_p.h
    freerdphelpers.h
    screenbuffer.h
    bitmaprectanglesink.h
    pointerchangesink.h
)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_DEFINITIONS REMOTEDISPLAY_LIBRARY)
target_link_libraries(${PROJECT_NAME}
    ${QT_LIBRARIES}
    freerdp-client
    freerdp-core
    freerdp-codec
    freerdp-cache
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(FILES remotedisplaywidget.h global.h DESTINATION include/RemoteDisplay)
