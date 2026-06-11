cmake_minimum_required(VERSION 3.15)
project(compare_hypotheses)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ROOT)

# Include ROOT headers
include(${ROOT_USE_FILE})

# Declare an executable target
add_executable(compare_hypotheses)

# Add sources 
target_sources(compare_hypotheses
    PRIVATE
        src/compare_hypotheses.cpp
        src/main.cpp
)

# Link ROOT
target_include_directories(compare_hypotheses PRIVATE ${ROOT_INCLUDE_DIRS})
target_link_libraries(compare_hypotheses ${ROOT_LIBRARIES})

# Move the executable to bin
install(TARGETS compare_hypotheses
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}) # this var is passed by scikit-build-core
