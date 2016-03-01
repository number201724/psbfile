# Install script for directory: /Users/yuanrui/Code/krkr_psbfile/jsoncpp/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/include/json/assertions.h;/usr/local/include/json/autolink.h;/usr/local/include/json/config.h;/usr/local/include/json/features.h;/usr/local/include/json/forwards.h;/usr/local/include/json/json.h;/usr/local/include/json/reader.h;/usr/local/include/json/value.h;/usr/local/include/json/version.h;/usr/local/include/json/writer.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/include/json" TYPE FILE FILES
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/assertions.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/autolink.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/config.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/features.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/forwards.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/json.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/reader.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/value.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/version.h"
    "/Users/yuanrui/Code/krkr_psbfile/jsoncpp/include/json/writer.h"
    )
endif()

