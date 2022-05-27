# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2022 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Copyright 2020 Metrological
# Licensed under the Apache License, Version 2.0
#

project(AVS)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(WPEFramework REQUIRED)

find_package(${NAMESPACE}Definitions REQUIRED)
find_package(GStreamer REQUIRED)
find_package(AlexaClientSDK REQUIRED)
find_package(AlexaSmartScreenSDK REQUIRED)

find_package(Asio REQUIRED)

set(AVS_NAME "AVS" CACHE STRING "The component name")
set(AVS_PLATFORM "rpi3" CACHE STRING "Platform name (currently only rpi3)")
set(AVS_DATA_PATH "/usr/share/WPEFramework/" CACHE STRING "Path were AVS data will be stored")
set(AVS_ALEXA_CLIENT_CONFIG "${AVS_DATA_PATH}/${AVS_NAME}/AlexaClientSDKConfig.json" CACHE STRING "Path to AlexaClientSDKConfig")
set(AVS_SMART_SCREEN_CONFIG "${AVS_DATA_PATH}/${AVS_NAME}/SmartScreenSDKConfig.json" CACHE STRING "Path to SmartScreenSDKConfig")
set(AVS_LOG_LEVEL "DEBUG9" CACHE STRING "Default log level for the SDK")
set(AVS_ENABLE_SMART_SCREEN_SUPPORT ON CACHE BOOL "Compile in the Smart Screen support")


# TODO: remove me ;)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")

set(LIBRARY_NAME xr-speech-avs)
string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)

if(GSTREAMER_FOUND)
    add_definitions(-DGSTREAMER_MEDIA_PLAYER)
endif()

add_definitions(-DRAPIDJSON_HAS_STDSTRING)

set(AVS_SMARTSCREEN_SOURCES)
list(APPEND AVS_SMARTSCREEN_SOURCES
        AVS.cpp
	./avs_sdt/avs_sdt.c
	./Impl/ThunderInputManager.cpp
	./Impl/ThunderLogger.cpp
	./Impl/SmartScreen/SmartScreen.cpp
)

add_library(${LIBRARY_NAME}
    SHARED
        ${AVS_SMARTSCREEN_SOURCES})

set_target_properties(${LIBRARY_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES)


if(AVS_ENABLE_SMART_SCREEN_SUPPORT)
    add_definitions(-DENABLE_SMART_SCREEN_SUPPORT)
endif()

if(AVS_ALEXA_CLIENT_CONFIG)
    add_definitions(-DALEXA_CLIENT_CONFIG="${AVS_ALEXA_CLIENT_CONFIG}")
endif()

if(AVS_SMART_SCREEN_CONFIG)
    add_definitions(-DSMART_SCREEN_CONFIG="${AVS_SMART_SCREEN_CONFIG}")
endif()

if(AVS_LOG_LEVEL)
    add_definitions(-DLOG_LEVEL="${AVS_LOG_LEVEL}")
endif()

add_subdirectory("Integration")

target_include_directories(${LIBRARY_NAME} PUBLIC
    "${AVSDSDK_INCLUDE_DIRS}"
    "${THUNDER_INCLUDE_DIRS}"
     Impl/
     ${ALEXA_CLIENT_SDK_INCLUDES}
     ${ALEXA_SMART_SCREEN_SDK_INCLUDES})

target_link_libraries(${LIBRARY_NAME}
    PRIVATE
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${ALEXA_CLIENT_SDK_LIBRARIES}
        ${ALEXA_SMART_SCREEN_SDK_LIBRARIES})

if(GSTREAMER_FOUND)
    target_include_directories(${LIBRARY_NAME} PUBLIC ${GSTREAMER_INCLUDES})
    target_link_libraries(${LIBRARY_NAME}
        PRIVATE
            ${GSTREAMER_LIBRARIES}
            MediaPlayer)
    add_definitions(-DGSTREAMER_MEDIA_PLAYER)
endif()

if(ASIO_FOUND)
    target_include_directories(${LIBRARY_NAME} PRIVATE Asio::Asio)
    add_definitions(-DASIO_STANDALONE)
else()
    message(FATAL_ERROR "MISSING Asio (Standalone) Library")
endif()

install(TARGETS ${LIBRARY_NAME}
    DESTINATION lib/)

