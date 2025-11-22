# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(POT)

# Custom target to erase the hmi partition
add_custom_target(erase_hmi_partition
    COMMAND ${CMAKE_COMMAND} -E echo "Erasing hmi partition..."
    COMMAND ${IDF_PATH}/components/esptool_py/esptool/esptool.py --chip esp32 --port ${ESPPORT} --baud ${ESPBAUD} erase_region 0x213000 0x6000
)

# Make the flash target depend on erase_hmi_partition
add_dependencies(flash erase_hmi_partition)
