idf_component_register(SRCS "telescope.cpp" "uartStream.cpp" "uart.cpp" "webserver.cpp" "main.cpp" "stepper.cpp"
                    INCLUDE_DIRS "."
                    EMBED_FILES "index.html" "core.css" "scope.js")

component_compile_options(-std=c++17)