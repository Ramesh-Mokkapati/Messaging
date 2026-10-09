cmake_minimum_required(VERSION 3.23)
project(OpenDDS_PubSub_Cxx11 CXX)

# --- C++11 for our own code. The DDS type's C++11 *language mapping*
#     (requested via OPENDDS_IDL_OPTIONS "-Lc++11" below) is a separate,
#     independent setting from this C++ standard version. ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate the OpenDDS CMake package. Requires either:
#  - having sourced/run setenv.cmd from your OpenDDS build, or
#  - passing -DCMAKE_PREFIX_PATH=<path-to-OpenDDS-install-or-build>
#    when you configure this project.
find_package(OpenDDS REQUIRED)

# ------------------------------------------------------------------
# Common: DDS type support generated once from Messenger.idl, shared
# by both executables (mirrors the MessengerTypesLib.vcxproj project
# in the Visual Studio solution).
# ------------------------------------------------------------------
add_library(MessengerTypesLib STATIC)
opendds_target_sources(MessengerTypesLib
  Common/Messenger.idl
  OPENDDS_IDL_OPTIONS "-Lc++11"
)
target_link_libraries(MessengerTypesLib PUBLIC OpenDDS::Dcps)
target_include_directories(MessengerTypesLib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# ------------------------------------------------------------------
# Publisher
# ------------------------------------------------------------------
add_executable(publisher Publisher/Publisher.cpp)
target_link_libraries(publisher PRIVATE
  MessengerTypesLib
  OpenDDS::Rtps
  OpenDDS::Rtps_Udp
)

# ------------------------------------------------------------------
# Subscriber
# ------------------------------------------------------------------
add_executable(subscriber
  Subscriber/Subscriber.cpp
  Subscriber/DataReaderListenerImpl.h
  Subscriber/DataReaderListenerImpl.cpp
)
target_link_libraries(subscriber PRIVATE
  MessengerTypesLib
  OpenDDS::Rtps
  OpenDDS::Rtps_Udp
)

# ------------------------------------------------------------------
# Copy the DDS transport/discovery config file next to each .exe so
# that running/debugging directly from Visual Studio "just works"
# (VS uses the target's output directory as the working directory).
# ------------------------------------------------------------------
foreach(tgt publisher subscriber)
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/rtps.ini"
            "$<TARGET_FILE_DIR:${tgt}>/rtps.ini"
  )
  set_target_properties(${tgt} PROPERTIES
    VS_DEBUGGER_COMMAND_ARGUMENTS "-DCPSConfigFile rtps.ini"
  )
endforeach()
