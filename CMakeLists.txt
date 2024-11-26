# CMakeList.txt : CMake project for FIGIMappingTool, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("FIGIMappingTool")

add_definitions(-DUNICODE -D_UNICODE)

# Add source to this project's executable.
add_executable (
	FIGIMappingTool 
	"FIGIMappingTool/main.cpp"  
	"FIGIMappingTool/Request.h"
	"FIGIMappingTool/Request.cpp"
	"FIGIMappingTool/states/FileState.h" 
	"FIGIMappingTool/states/FileState.cpp" 
	"FIGIMappingTool/utilities/Timer.h" 
	"FIGIMappingTool/utilities/Timer.cpp" 
	"FIGIMappingTool/utilities/utils.h" 
	"FIGIMappingTool/utilities/utils.cpp" 
	"FIGIMappingTool/utilities/jsonparse.h" 
	"FIGIMappingTool/utilities/jsonparse.cpp" 
	"FIGIMappingTool/states/WindowState.h"
	"FIGIMappingTool/states/WindowState.cpp"
	"FIGIMappingTool/states/BaseWindow.h"
    "FIGIMappingTool/FIGIMappingTool.rc"  # Include the resource file here
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET FIGIMappingTool PROPERTY CXX_STANDARD 20)
endif()

# Set the executable as a Windows GUI application
set_target_properties(FIGIMappingTool PROPERTIES WIN32_EXECUTABLE TRUE)

# Add the json_lib library
add_library(json_lib INTERFACE)

# Specify the include directories for tinyxml2
target_include_directories(json_lib INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/FIGIMappingTool/libs/json"
)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/FIGIMappingTool/libs/cpr")

# Link against comctl32 library
target_link_libraries(FIGIMappingTool PRIVATE comctl32)

# Add the libraries to the target
target_link_libraries(FIGIMappingTool PRIVATE json_lib)
target_link_libraries(FIGIMappingTool PRIVATE cpr::cpr)

# TODO: Add tests and install targets if needed.