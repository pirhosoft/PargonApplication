set(MODULE_NAME Application)

set(INCLUDES
	Application.h
	Application.Win32.h
)

set(HEADERS
	Application.h
	Clipboard.h
	EventStream.h
	Loader.h
	Log.h
)

set(SOURCES
	Core/Application.cpp
	Core/Clipboard.cpp
	Core/Loader.cpp
	Core/Log.cpp
)

set(MODULES
	Containers
	Serialization
	Types
)

set(WIN32_SOURCES
	Win32/Application.Win32.cpp
	Win32/Clipboard.Win32.cpp
	Win32/Log.Win32.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL Windows)
	list(APPEND SOURCES ${WIN32_SOURCES})
endif()

set(TARGET_NAME ${MODULE_PREFIX}${MODULE_NAME})
set(INCLUDE_LOCATION Include/Pargon/)
set(HEADER_LOCATION ${INCLUDE_LOCATION}${MODULE_NAME}/)
set(SOURCE_LOCATION Source/)

list(TRANSFORM INCLUDES PREPEND ${INCLUDE_LOCATION})
list(TRANSFORM HEADERS PREPEND ${HEADER_LOCATION})
list(TRANSFORM SOURCES PREPEND ${SOURCE_LOCATION})
list(TRANSFORM MODULES PREPEND ${MODULE_PREFIX})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_LOCATION}" FILES ${INCLUDES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER_LOCATION}" PREFIX Include FILES ${HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_LOCATION}" FILES ${SOURCES})

add_library(${TARGET_NAME} STATIC)
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
target_include_directories(${TARGET_NAME} PUBLIC Include)
target_include_directories(${TARGET_NAME} PRIVATE Source)
target_sources(${TARGET_NAME} PRIVATE "${INCLUDES}" "${HEADERS}" "${SOURCES}")
target_link_libraries(${TARGET_NAME} PUBLIC ${MODULES})
