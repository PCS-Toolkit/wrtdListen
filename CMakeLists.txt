CMAKE_MINIMUM_REQUIRED(VERSION 3.16 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

PROJECT(wrtdListen VERSION 1.0.0 LANGUAGES C)

set(CMAKE_MACOSX_RPATH 1)
#set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE) 
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")  #fixed
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib") #relative relocatable (linux only.. macos different)

set(libwrtd_VERSION 1.0.1)

include(FetchContent)
cmake_policy(SET CMP0097 NEW) # only in 3.16 (which is default for ubuntu 20.04)
FetchContent_Declare(
    wrtd_common
    GIT_REPOSITORY https://ohwr.org/project/wrtd.git
    GIT_TAG v${libwrtd_VERSION}
    GIT_SUBMODULES "doc" # Non-git-submodule dir that already exists work around for bug in cmake < 3.20?
)
FetchContent_MakeAvailable(wrtd_common)
set(wrtd_INCLUDE ${wrtd_common_SOURCE_DIR}/software/include)

find_package(POPT)

ADD_EXECUTABLE(wrtdWait
               wrtdMain.c)

ADD_LIBRARY(
    ${PROJECT_NAME} SHARED
    wrtdListen.c
)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER wrtdListen.h)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PRIVATE
        ${wrtd_INCLUDE}
)

TARGET_LINK_LIBRARIES(wrtdWait
    PRIVATE
    ${PROJECT_NAME}
    popt
)

install(TARGETS wrtdWait)
install(TARGETS wrtdListen)

install(FILES "${wrtd_INCLUDE}/wrtd-common.h" TYPE INCLUDE)