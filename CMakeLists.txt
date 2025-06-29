
# define minimum cmake version => always necessary
cmake_minimum_required(VERSION 3.0)

# define project name
project(chess)

# require c++17 to have filesystem
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include libraries
include_directories(lib/eigen) # Eigen is a header only library => no need for target_link_libraries
include_directories(lib/tintoretto)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# !-- Add inc and src files --! #
include_directories(("${CMAKE_SOURCE_DIR}/inc"))
file(GLOB_RECURSE SOURCES "src/*.cpp")

# !-- Define target --! #
# name of the script that we want to compile inside the app folder
if(DEFINED ENV{app_cpp})
    set(SCRIPT_NAME $ENV{app_cpp})
else()
    set(SCRIPT_NAME "main.cpp")
endif()

# ----------------- #

# let's adapt the name of the exectutable to this
# extract base name without extension
get_filename_component(EXECUTABLE_NAME ${SCRIPT_NAME} NAME_WE) # main.cpp --> main
# add .exe extension
set(EXECUTABLE_NAME ${EXECUTABLE_NAME}.exe) # main --> main.exe

# ----------------- #

# define which script we want to execute
add_executable(${EXECUTABLE_NAME} app/${SCRIPT_NAME} ${SOURCES})

# link libraries to executable
target_link_libraries(${EXECUTABLE_NAME} sfml-graphics sfml-window sfml-system)

# say where we want to create our executable
set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Add a custom command to run the executable after build
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD # run once the build is done
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "!-- Running App --!"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND $<TARGET_FILE:${EXECUTABLE_NAME}>
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "!-- App exited --!"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMENT "Running ${EXECUTABLE_NAME} after build"
)

# then do form root:
# cd build
# cmake ..
# make

