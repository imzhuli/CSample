project(sample_${libname} C CXX)

# local dependencies
include_directories(
	../lib/src/
	../lib/include/
	)
link_directories(
	)

set(shared_lib_dependency_private
)

function(add_sub_test sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(test_${sub_dir_name}  ${LOCAL_SOURCE_CODE})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(test_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(test_${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
	set(extra_args ${ARGV})
	list(POP_FRONT extra_args)
	add_test(
		NAME	test_${sub_dir_name}
		COMMAND test_${sub_dir_name} ${extra_args}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		)
endfunction()

#sample lib:

add_sub_test(_cpp_compat)
add_sub_test(getopt -a "yes" -bc "check" -- "comma-error" -c "again")
add_sub_test(indexed_storage)
add_sub_test(list)
add_sub_test(ip_address)
add_sub_test(threads)
