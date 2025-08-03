
file(GLOB shader_files "${SOURCE_DIR}/src/util/Shaders/*")

file(MAKE_DIRECTORY "${TARGET_DIR}/Shaders")

foreach(shader_file IN LISTS shader_files)
    get_filename_component(filename ${shader_file} NAME)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${shader_file}" "${TARGET_DIR}/Shaders/${filename}"
    )
endforeach()