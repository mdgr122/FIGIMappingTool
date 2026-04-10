cmake_minimum_required(VERSION 3.25)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("FIGIMappingTool" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DUNICODE -D_UNICODE)

# Add source to this project's executable.
add_executable (
	FIGIMappingTool 
	"FIGIMappingTool/main.cpp"  
	"FIGIMappingTool/core/Request.h"
	"FIGIMappingTool/core/Request.cpp"
	"FIGIMappingTool/states/FileState.h" 
	"FIGIMappingTool/states/FileState.cpp" 
	"FIGIMappingTool/utilities/Timer.h" 
	"FIGIMappingTool/utilities/Timer.cpp" 
	"FIGIMappingTool/utilities/Utils.h" 
	 
	"FIGIMappingTool/utilities/jsonparse.h" 
	"FIGIMappingTool/utilities/jsonparse.cpp" 
	"FIGIMappingTool/states/WindowState.h"
	"FIGIMappingTool/states/WindowState.cpp"
	"FIGIMappingTool/states/BaseWindow.h"
    "FIGIMappingTool/FIGIMappingTool.rc"
 "FIGIMappingTool/core/OpenFigiClient.h" "FIGIMappingTool/core/OpenFigiClient.cpp")

# Set the executable as a Windows GUI application
set_target_properties(FIGIMappingTool PROPERTIES WIN32_EXECUTABLE TRUE)
target_compile_definitions(FIGIMappingTool PRIVATE NOMINMAX)

# JSON header-only library
add_library(json_lib INTERFACE)
target_include_directories(json_lib INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/FIGIMappingTool/libs/json"
)

# CPR HTTP library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/FIGIMappingTool/libs/cpr")

target_link_libraries(FIGIMappingTool PRIVATE comctl32 json_lib cpr::cpr)