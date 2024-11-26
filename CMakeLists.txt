# CMakeList.txt : CMake project for OpenFIGI, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("OpenFIGI")

add_definitions(-DUNICODE -D_UNICODE)

# Add source to this project's executable.
add_executable (
	OpenFIGI 
	"OpenFIGI/main.cpp"  
	"OpenFIGI/Request.h"
	"OpenFIGI/Request.cpp"
	"OpenFIGI/states/FileState.h" 
	"OpenFIGI/states/FileState.cpp" 
	"OpenFIGI/utilities/Timer.h" 
	"OpenFIGI/utilities/Timer.cpp" 
	"OpenFIGI/utilities/utils.h" 
	"OpenFIGI/utilities/utils.cpp" 
	"OpenFIGI/utilities/jsonparse.h" 
	"OpenFIGI/utilities/jsonparse.cpp" 
	"OpenFIGI/states/WindowState.h"
	"OpenFIGI/states/WindowState.cpp"
	"OpenFIGI/states/BaseWindow.h"
    "OpenFIGI/OpenFIGI.rc"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OpenFIGI PROPERTY CXX_STANDARD 20)
endif()

# Set the executable as a Windows GUI application
set_target_properties(OpenFIGI PROPERTIES WIN32_EXECUTABLE TRUE)

# Add the json_lib library
add_library(json_lib INTERFACE)

# Specify the include directories for tinyxml2
target_include_directories(json_lib INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/OpenFIGI/libs/json"
)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/OpenFIGI/libs/cpr")

# Link against comctl32 library
target_link_libraries(OpenFIGI PRIVATE comctl32)

# Add the libraries to the target
target_link_libraries(OpenFIGI PRIVATE json_lib)
target_link_libraries(OpenFIGI PRIVATE cpr::cpr)

# TODO: Add tests and install targets if needed.