CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12.1)

SET(CMAKE_CXX_STANDARD 17)
PROJECT(3DHumanCapture)

FIND_PACKAGE(freenect2 REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(rpclib REQUIRED)
FIND_PACKAGE(Boost REQUIRED thread)
FIND_PACKAGE(Threads REQUIRED)

SET(GLAD_SRC src/common/glad.c)

# Adding NanoGUI since it does not provide a proper CMake definition:
SET(FIND_NANOGUI_PATHS
  $ENV{NANOGUI_ROOT}
  ${CMAKE_CURRENT_LIST_DIR}/..
  ${CMAKE_CURRENT_LIST_DIR}/../build/output/lib
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /opt/local
  /opt)

FIND_LIBRARY(NANOGUI_LIBS
  NAMES libnanogui nanogui
  PATHS ${FIND_NANOGUI_PATHS})

# HACK:rpclib because Findrpclib does not set RPCLIB_LIBS
SET(FIND_RPCLIB_PATHS
    $ENV{RPCLIB_ROOT}
    ${CMAKE_CURRENT_LIST_DIR}/..                   # To support in-tree build
    ${CMAKE_CURRENT_LIST_DIR}/../build/output/lib  #
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local
    /usr
    /opt/local
    /opt)

FIND_LIBRARY(RPCLIB_LIBS
    NAMES librpc rpclib rpc
    PATHS ${FIND_RPCLIB_PATHS})
# end of HACK:rpclib

INCLUDE_DIRECTORIES(
  "${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/include/client"
	"${PROJECT_SOURCE_DIR}/include/server"
	"${PROJECT_SOURCE_DIR}/include/common"
  ${NANOGUI_INCLUDE_DIR}
	${freenect2_INCLUDE_DIR}
	${OpenCV_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${RPCLIB_INCLUDE_DIR}
  ${NANOGUI_INCLUDE_DIR}
)

# Client Binary:
ADD_EXECUTABLE(Client
  src/client/Camera.cpp
  src/client/Main.cpp
  ${GLAD_SRC}
)
TARGET_LINK_LIBRARIES(Client 
  ${OpenCV_LIBS}
  ${freenect2_LIBRARIES}
  Boost::thread
  ${RPCLIB_LIBS}
  -ldl
)

# Server Binary:
ADD_EXECUTABLE(Server
    src/common/type_definitions.cpp
    src/server/data_capture.cpp
    src/server/main.cpp
    ${GLAD_SRC})
TARGET_LINK_LIBRARIES(Server ${RPCLIB_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${NANOGUI_LIBS} -ldl)

# Calibration Binary:
ADD_EXECUTABLE(calibrate
  src/calibration/calibrate.cpp
  src/client/Camera.cpp
  src/calibration/calibration_window.cpp
  ${GLAD_SRC}
)
target_link_libraries(
  calibrate
  -ldl
  glfw
  ${freenect2_LIBRARIES}
  ${OpenCV_LIBS}
)
