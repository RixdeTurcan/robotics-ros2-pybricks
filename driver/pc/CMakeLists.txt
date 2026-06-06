cmake_minimum_required(VERSION 3.10)
project(pybricks-driver VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PYBRICKS_DRIVER_WITHOUT_MSGPACK OFF)
set(PYBRICKS_DRIVER_WITHOUT_SIMPLEBLE OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

add_compile_definitions(
	PYBRICKS_DRIVER_LOG_PATH="${CMAKE_LOG_PATH}/pybricks-driver/pybricks-driver.log"
	PYBRICKS_LOG_LEVEL=4
)


find_package(cppzmq REQUIRED)
find_package(spdlog REQUIRED)
find_package(simpleble REQUIRED)
find_package(msgpack-cxx REQUIRED)

####Library pybricks-driver Core####

add_library(pybricks-driver-core
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/events.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/commands.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/sensordata.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/actuatordata.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/commands_pubsub.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/constant.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/constant_internal.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/messaging.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/tools/logging.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/tools/tools.cpp
)
add_library(pybricks-driver::core ALIAS pybricks-driver-core)

set_target_properties(pybricks-driver-core PROPERTIES EXPORT_NAME core)

target_link_libraries(pybricks-driver-core
	PUBLIC
	spdlog::spdlog
	simpleble::simpleble
)

target_include_directories(pybricks-driver-core
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(pybricks-driver-core
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/types.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/constant.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/msgpack_compat.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/simpleble_compat.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/commands_pubsub.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/commands.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/sensordata.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/actuatordata.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/events.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/logging.h
)

install(TARGETS pybricks-driver-core
	EXPORT pybricks-driverTargets
	FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


####Library pybricks-driver PubSub####

add_library(pybricks-driver-pubsub
	src/PubSub/PubSub.cpp
)
add_library(pybricks-driver::pubsub ALIAS pybricks-driver-pubsub)

set_target_properties(pybricks-driver-pubsub PROPERTIES EXPORT_NAME pubsub)

target_link_libraries(pybricks-driver-pubsub
	PUBLIC
	cppzmq
	msgpack-cxx
	pybricks-driver::core
)

target_include_directories(pybricks-driver-pubsub
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(pybricks-driver-pubsub
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
	FILES
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/PubSub.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/pybricks-driver/PubSub.hpp
)

install(TARGETS pybricks-driver-pubsub
	EXPORT pybricks-driverTargets
	FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

####Executable pybricks-driver####

add_executable(pybricks-driver
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/src/HubDriver/HubDriver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/BLEManager/BLEManager.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/DriverFSM/DriverFSM.cpp
)

target_link_libraries(pybricks-driver
	PRIVATE
	pybricks-driver::core
	pybricks-driver::pubsub
)

target_include_directories(pybricks-driver
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

install(TARGETS pybricks-driver
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

####Generate cmake configs####

install(EXPORT pybricks-driverTargets
	FILE pybricks-driverTargets.cmake
	NAMESPACE pybricks-driver::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pybricks-driver
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/pybricks-driverConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/pybricks-driverConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/pybricks-driverConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/pybricks-driver"
)

install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/pybricks-driverConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/pybricks-driverConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/pybricks-driver"
)