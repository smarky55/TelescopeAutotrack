
idf_component_register(SRCS "./src/CanonM50.cpp"
                            "./src/ConnectivityState.cpp"
                            "./src/IntervalometerTask.cpp"
                            "./src/Scan.cpp"
                       INCLUDE_DIRS "./include"
                       REQUIRES "esp-nimble-cpp")

component_compile_options(-std=c++17)