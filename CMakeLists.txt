cmake_minimum_required(VERSION 3.10)

project(lumia-dloadtool)

file(GLOB SOURCE_FILES "source/*.c")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} Cfgmgr32 Winusb)
    target_include_directories(${PROJECT_NAME} PUBLIC "include")
else()
    find_package(PkgConfig)
    pkg_check_modules(libusb REQUIRED libusb-1.0)

    target_link_directories(${PROJECT_NAME} PUBLIC ${libusb_LIBRARY_DIRS})
    target_link_libraries(${PROJECT_NAME} ${libusb_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PUBLIC "include" ${libusb_INCLUDE_DIRS})
endif()
