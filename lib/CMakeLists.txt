project(lib_${libname} C)

find_package(Threads REQUIRED)

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(PLATFORM_LIBRARIES wsock32 ws2_32)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_LIBRARIES)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	set(PLATFORM_LIBRARIES ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
endif()

# local dependencies
# paths
include_directories(
	./include/
	)
link_directories(
	)

# libs:
set(shared_lib_dependency_private
	)
set(shared_lib_dependency_public
	Threads::Threads
	${PLATFORM_LIBRARIES}
	)

# source files:
aux_source_directory(./src/xel                          X_SRC)
aux_source_directory(./src/xel/Util                     X_SRC)
aux_source_directory(./src/xel/Threads                  X_SRC)
aux_source_directory(./src/xel_ext/net                  X_SRC)

# 3rd party

# C version utils:

if (X_OPTION_STATIC)
#static lib:
	add_library(${libname} ${X_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE X_OPTION_STATIC)

else()
#dynamic lib:
	add_library(${libname} SHARED  ${X_SRC})
	set_target_properties(${libname} PROPERTIES VERSION 1.0)
	target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})
	target_compile_definitions(${libname} PRIVATE X_OPTION_EXPORT_API)
endif()

#install
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/xel
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)
install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/./include/xel_ext
	DESTINATION include
	FILES_MATCHING REGEX ".+\.(h|hpp)"
	)
install(TARGETS
	${libname}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
