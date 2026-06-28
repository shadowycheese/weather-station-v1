# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/alan/.espressif/v5.5.4/esp-idf/components/bootloader/subproject"
  "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader"
  "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader-prefix"
  "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader-prefix/tmp"
  "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader-prefix/src/bootloader-stamp"
  "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader-prefix/src"
  "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/alan/projects/weather-station/weather-station-v1/shared_components/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
