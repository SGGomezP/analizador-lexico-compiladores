cmake_minimum_required(VERSION 3.15)
project(LexLP CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lexlp
    src/main.cpp
    src/Lexer.cpp
    src/Token.cpp
    src/SymbolTable.cpp
)

# Para que al ejecutar desde Visual Studio encuentre tests/ y output/
# sin tener que escribir rutas completas.
set_target_properties(lexlp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
