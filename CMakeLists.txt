cmake_minimum_required(VERSION 3.21)

# standards & flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")

include(GNUInstallDirs)

# info
project(
	ModernStaggerLock
	VERSION 1.0.7
	LANGUAGES CXX
)

# out-of-source builds only
if(${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# in-place configuration
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

# source files
execute_process(COMMAND powershell -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/!Update.ps1" "SOURCEGEN" "${PROJECT_VERSION}" "${CMAKE_CURRENT_BINARY_DIR}")
include(${CMAKE_CURRENT_BINARY_DIR}/sourcelist.cmake)
source_group(
	TREE 
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES 
		${SOURCES}
)

source_group(
	TREE ${CMAKE_CURRENT_BINARY_DIR}
	FILES ${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# dependencies
add_subdirectory($ENV{CommonLibSSEPath} CommonLibSSE)
add_subdirectory($ENV{DKUtilPath} DKUtil)
find_package(spdlog CONFIG REQUIRED)

# cmake target
# runtime
add_library(
	${PROJECT_NAME}
	SHARED
		${SOURCES}
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
		${CMAKE_CURRENT_BINARY_DIR}/version.rc
		.clang-format
		vcpkg.json
)

# include dir
target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

# linkage
target_link_libraries(
	${PROJECT_NAME} 
	PRIVATE
		CommonLibSSE::CommonLibSSE
		DKUtil::DKUtil
		spdlog::spdlog
)

# compiler def
if (MSVC)
	add_compile_definitions(_UNICODE)

	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/MP
			/await
			/W0
			/WX
			/permissive-
			/Zc:alignedNew
			/Zc:auto
			/Zc:__cplusplus
			/Zc:externC
			/Zc:externConstexpr
			/Zc:forScope
			/Zc:hiddenFriend
			/Zc:implicitNoexcept
			/Zc:lambda
			/Zc:noexceptTypes
			/Zc:preprocessor
			/Zc:referenceBinding
			/Zc:rvalueCast
			/Zc:sizedDealloc
			/Zc:strictStrings
			/Zc:ternary
			/Zc:threadSafeInit
			/Zc:trigraphs
			/Zc:wchar_t
			/wd4200 # nonstandard extension used : zero-sized array in struct/union
	)
endif()

# PCH
target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		src/PCH.h
)

# post build event
add_custom_command(
	TARGET 
		${PROJECT_NAME}
	POST_BUILD
		COMMAND 
			powershell -NoProfile -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/!Update.ps1" "COPY" "${PROJECT_VERSION}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$(ConfigurationName)" "${PROJECT_NAME}" "${ANNIVERSARY_EDITION}"
		COMMENT 
			"Updating project :))"
)

# called via !Rebuild.ps1
if(DEFINED GROUP)
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
	FOLDER
	${GROUP}
)
endif()
