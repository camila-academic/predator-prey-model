cmake_minimum_required(VERSION 3.20)

project(game C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.c)
add_executable(game ${SRC})

option(MO_GUI "Desativa a compilação do modo gráfico" OFF)

if(MO_GUI)
    target_compile_definitions(game PRIVATE MO_GUI)
endif()


find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)


target_include_directories(game PRIVATE
        include
        ${RAYLIB_INCLUDE_DIRS}
)

target_link_libraries(game PRIVATE
        ${RAYLIB_LIBRARIES}
        m
)

target_compile_options(game PRIVATE
        -Wall
        -Wextra
)

# Copia os assets do jogo pra o diretório de build
file(GLOB_RECURSE PNG_FILES
        "${CMAKE_SOURCE_DIR}/assets/*.png"
)

foreach(PNG_FILE ${PNG_FILES})

    file(RELATIVE_PATH REL_PATH
            "${CMAKE_SOURCE_DIR}/assets"
            "${PNG_FILE}"
    )

    get_filename_component(
            REL_DIR
            "${REL_PATH}"
            DIRECTORY
    )

    add_custom_command(
            TARGET game POST_BUILD

            COMMAND ${CMAKE_COMMAND} -E make_directory
            "${CMAKE_SOURCE_DIR}/bin/assets/${REL_DIR}"

            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PNG_FILE}"
            "${CMAKE_SOURCE_DIR}/bin/assets/${REL_PATH}"

            VERBATIM
    )

endforeach()
