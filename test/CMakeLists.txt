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


function(add_sub_test sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(test_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(test_${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
	add_test(
		NAME	test_${sub_dir_name}
		COMMAND test_${sub_dir_name}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		)
endfunction()

#sample lib:

add_sub_test(indexed_storage)

