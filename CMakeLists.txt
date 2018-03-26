CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(3DHumanCapture)

set (CMAKE_CXX_STANDARD 14)

FIND_PACKAGE(freenect2 REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(rpclib REQUIRED)
FIND_PACKAGE(Boost REQUIRED thread system filesystem)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(yaml-cpp REQUIRED)

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
  ${YAML_CPP_INCLUDE_DIR}
  ${NANOGUI_INCLUDE_DIR}
)

ADD_LIBRARY(Utils
    src/common/utils.cpp)
TARGET_LINK_LIBRARIES(Utils
    ${YAML_CPP_LIBRARIES})

# Client Binary:
ADD_EXECUTABLE(Client 
    src/client/timeService.cpp
    src/client/Camera.cpp 
    src/client/Main.cpp
    ${GLAD_SRC})
TARGET_LINK_LIBRARIES(Client 
    ${OpenCV_LIBS} 
    glfw 
    ${freenect2_LIBRARIES} 
    ${Boost_LIBRARIES} 
    ${RPCLIB_LIBS} 
    ${CMAKE_DL_LIBS}
    Utils
    ${CMAKE_THREAD_LIBS_INIT})
ADD_DEPENDENCIES(Client Utils)

# Server Binary:
ADD_EXECUTABLE(Server
    src/common/type_definitions.cpp
    src/server/data_capture.cpp
    src/server/frame_processors.cpp
    src/server/frontend.cpp
    src/server/pack_of_frames_to_disk_processor.cpp
    src/server/pack_of_frames_frontend_processor.cpp
    src/server/main.cpp
    ${GLAD_SRC})
TARGET_LINK_LIBRARIES(Server 
    ${RPCLIB_LIBS} 
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${OpenCV_LIBS}
    ${CMAKE_DL_LIBS}
    Utils)
ADD_DEPENDENCIES(Server Utils)
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
  glfw
  ${CMAKE_DL_LIBS}
  ${freenect2_LIBRARIES}
  ${OpenCV_LIBS}
)
