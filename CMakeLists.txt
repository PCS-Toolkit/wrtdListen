CMAKE_MINIMUM_REQUIRED(VERSION 3.16 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

PROJECT(wrtdListen VERSION 1.0.0 LANGUAGES C)

FILE(GLOB _SOURCE_LIST *.c *.h)
#ADD_EXECUTABLE(
#    ${PROJECT_NAME}
#    ${_SOURCE_LIST}
#)

include(ExternalProject)
cmake_policy(SET CMP0097 NEW) # only in 3.16 (which is default for ubuntu 20.04)
ExternalProject_Add(wrtd
    # this package is HUGE with all kinds of HDL in submodules.. 
    # need to hack at cmake to get only the part we want.
    # so doing tricks to just get the libwrtd stuff
    GIT_REPOSITORY https://ohwr.org/project/wrtd.git
    GIT_TAG v1.0.1
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_SUBMODULES "" # need mockturtle.. but not recursively, so add to fake configure command
    UPDATE_COMMAND git fetch --tags
    COMMAND git checkout ${GIT_TAG}
    COMMAND git submodule update --init dependencies/mock-turtle
    CONFIGURE_COMMAND ""
    SOURCE_SUBDIR   software/lib
    BUILD_IN_SOURCE TRUE
    BUILD_COMMAND   export EXTRACFLAGS=-Wno-format-truncation
    COMMAND         make -e
    INSTALL_COMMAND make PREFIX=<INSTALL_DIR> install
    COMMAND         install -d <INSTALL_DIR>/include  # ugh.. makefile doesn't install the header
    COMMAND         install -m 0755 <SOURCE_DIR>/software/include/wrtd-common.h <INSTALL_DIR>/include
)
ExternalProject_Get_Property(wrtd INSTALL_DIR)

find_package(POPT)

ADD_EXECUTABLE(wrtdWait
               wrtdMain.c)

ADD_LIBRARY(
    ${PROJECT_NAME} SHARED
    wrtdListen.c
)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER wrtdListen.h)
add_dependencies(${PROJECT_NAME} wrtd)

TARGET_INCLUDE_DIRECTORIES(
    ${PROJECT_NAME}
    PRIVATE
        ${INSTALL_DIR}/include
)

TARGET_LINK_LIBRARIES(wrtdWait
    PRIVATE
      ${PROJECT_NAME}
      popt
)

install(TARGETS wrtdWait)
install(TARGETS wrtdListen)
