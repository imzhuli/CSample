project(sample_${libname} C)

# local dependencies
include_directories(
	../lib/src/
	../lib/include/
	${LUA_INCLUDE_DIR}
	)
link_directories(
	)

set(shared_lib_dependency_private
	${LUA_LIBRARIES}
)

function(add_sub_sample sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(sample_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(sample_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(sample_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(sample_${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
endfunction()

#sample lib:

add_sub_sample(hello_world)

install(TARGETS
	sample_hello_world
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
