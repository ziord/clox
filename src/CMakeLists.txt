
include_directories(include)

add_executable(clox
        compiler.c
        debug.c
        lexer.c
        memory.c
        table.c
        value.c
        vm.c
        object.c
        main.c
        chunk.c
        ${PROJECT_SOURCE_DIR}/include/chunk.h
        ${PROJECT_SOURCE_DIR}/include/common.h
        ${PROJECT_SOURCE_DIR}/include/compiler.h
        ${PROJECT_SOURCE_DIR}/include/debug.h
        ${PROJECT_SOURCE_DIR}/include/lexer.h
        ${PROJECT_SOURCE_DIR}/include/memory.h
        ${PROJECT_SOURCE_DIR}/include/object.h
        ${PROJECT_SOURCE_DIR}/include/table.h
        ${PROJECT_SOURCE_DIR}/include/value.h
        ${PROJECT_SOURCE_DIR}/include/vm.h
        )
