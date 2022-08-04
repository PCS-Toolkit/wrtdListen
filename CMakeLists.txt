CMAKE_MINIMUM_REQUIRED(VERSION 3.16 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

PROJECT(wrtdListen VERSION 1.0.0 LANGUAGES C)

FILE(GLOB _SOURCE_LIST *.c *.h)
#ADD_EXECUTABLE(
#    $w{PROJECT_NAME}
#    ${_SOURCE_LIST}
#)

set(CMAKE_MACOSX_RPATH 1)
#set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE) 
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")  #fixed
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib") #relative relocatable (linux only.. macos different)


include(ExternalProject)
cmake_policy(SET CMP0097 NEW) # only in 3.16 (which is default for ubuntu 20.04)
set(libwrtd_VERSION 1.0.1)
ExternalProject_Add(wrtd
    # this package is HUGE with all kinds of HDL in submodules.. 
    # need to hack at cmake to get only the part we want.
    # so doing tricks to just get the libwrtd stuff
    GIT_REPOSITORY https://ohwr.org/project/wrtd.git
    GIT_TAG v${libwrtd_VERSION}
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_SUBMODULES "" # need mockturtle.. but not recursively, so add to fake configure command
    UPDATE_COMMAND git fetch --tags
    COMMAND git checkout v${libwrtd_VERSION}
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
set(wrtd_INSTALL_DIR "${INSTALL_DIR}")
set(wrtd_INCLUDE "${INSTALL_DIR}/include")

add_library(libwrtd SHARED IMPORTED GLOBAL)
set_target_properties(libwrtd PROPERTIES
    IMPORTED_LOCATION "${INSTALL_DIR}/lib/libwrtd.so"
)


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
        ${wrtd_INCLUDE}
)

TARGET_LINK_LIBRARIES(wrtdWait
    PUBLIC 
    libwrtd
    PRIVATE
    ${PROJECT_NAME}
    popt
)

install(TARGETS wrtdWait)
install(TARGETS wrtdListen)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
    install(IMPORTED_RUNTIME_ARTIFACTS libwrtd)  #this might work in the future?
else()  # evil hack but it works, though it doesn't add rpath
    install(FILES
        "${wrtd_INSTALL_DIR}/lib/libwrtd.so" 
        "${wrtd_INSTALL_DIR}/lib/libwrtd.so.${libwrtd_VERSION}" 
        TYPE LIB)
    install(FILES "${wrtd_INSTALL_DIR}/include/wrtd-common.h" TYPE INCLUDE)
endif()