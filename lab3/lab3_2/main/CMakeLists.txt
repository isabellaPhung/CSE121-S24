set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
idf_component_register(SRCS "lab3_2.cpp" "DFRobot_LCD.cpp"
                    INCLUDE_DIRS ".")
