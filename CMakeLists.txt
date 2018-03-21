CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12.1)

SET(PROJECT_VER_MAJOR 0)
SET(PROJECT_VER_MINOR 0)
SET(PROJECT_VER_PATCH 1)

PROJECT(3DHumanCapture)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")


FIND_PACKAGE(freenect2 REQUIRED)
FIND_PACKAGE(glfw3 REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(rpclib REQUIRED)
FIND_PACKAGE(Boost REQUIRED thread)
FIND_PACKAGE(Threads REQUIRED)

# HACK:rpclib because Findrpclib does not set RPCLIB_LIBS
set(FIND_RPCLIB_PATHS
    $ENV{RPCLIB_ROOT}
    ${CMAKE_CURRENT_LIST_DIR}/..                   # To support in-tree build
    ${CMAKE_CURRENT_LIST_DIR}/../build/output/lib  #
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /opt/local
    /opt)

find_library(RPCLIB_LIBS
    NAMES librpc rpclib rpc
    PATHS ${FIND_RPCLIB_PATHS})
# end of HACK:rpclib

INCLUDE_DIRECTORIES(
        "${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/include/client"
	"${PROJECT_SOURCE_DIR}/include/server"
	"${PROJECT_SOURCE_DIR}/include/common"
	${freenect2_INCLUDE_DIR}
	${glfw3_INCLUDE_DIR}
	${OpenCV_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${RPCLIB_INCLUDE_DIR}
)

ADD_EXECUTABLE(Client src/client/Camera.cpp src/client/Main.cpp)
TARGET_LINK_LIBRARIES(Client ${OpenCV_LIBS} glfw ${freenect2_LIBRARIES} Boost::thread ${RPCLIB_LIBS})

ADD_EXECUTABLE(Server
    src/common/type_definitions.cpp
    src/server/data_capture.cpp
    src/server/main.cpp)
TARGET_LINK_LIBRARIES(Server ${RPCLIB_LIBS} ${CMAKE_THREAD_LIBS_INIT})

set(OPENGL_LIBRARY ${OPENGL_LIBRARY} -lglfw -lrt -lm -ldl -lGLEW -lGLU -lGL)
set(CALIBRATION_HEADERS ${CALIBRATION_HEADERS} include/client/Camera.h)
add_executable(calibrate src/calibration/calibrate.cpp src/client/Camera.cpp src/calibration/calibration_window.cpp include/calibration/calibration_window.h)
target_link_libraries(calibrate ${OpenCV_LIBS} glfw ${freenect2_LIBRARIES})
target_link_libraries(calibrate ${OPENGL_LIBRARY})
