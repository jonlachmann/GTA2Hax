cmake_minimum_required (VERSION 3.5)
project (GTA2Renderer)

SET(CMAKE_WARN_VS6 CACHE BOOL OFF)

set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_CURRENT_LIST_DIR}/cmake")

include_directories(Common)
include_directories(DmaVideo)
include_directories(3rdParty/imgui)

link_directories(Common)
add_library(DmaVideo SHARED 
  Common/types.hpp
  DmaVideo/Exports.def
  DmaVideo/dllmain.cpp
  DmaVideo/DmaVideo.cpp
  DmaVideo/DmaVideo.hpp
  )
target_compile_definitions(DmaVideo PRIVATE IMGUI_DLL)
target_link_libraries(DmaVideo dear_imgui)

add_library(d3ddll SHARED
  Common/types.hpp 
  d3ddll/Exports.def
  d3ddll/dllmain.cpp
  d3ddll/d3ddll.cpp
  d3ddll/d3ddll.hpp
  )
target_compile_definitions(d3ddll PRIVATE IMGUI_DLL)
target_link_libraries(d3ddll DmaVideo dear_imgui)

add_library(dear_imgui SHARED
  3rdParty/imgui/imgui.h
  3rdParty/imgui/imgui.cpp
  3rdParty/imgui/imgui_draw.cpp
  3rdParty/imgui/examples/directx9_example/imgui_impl_dx9.cpp
  )
target_compile_definitions(dear_imgui PRIVATE IMGUI_DLL IMGUI_EXPORTS)
target_link_libraries(dear_imgui)


add_executable(imgui_test
3rdParty/imgui/imgui.h
3rdParty/imgui/imgui.cpp
3rdParty/imgui/imgui_draw.cpp
3rdParty/imgui/examples/directx9_example/imgui_impl_dx9.cpp
3rdParty/imgui/examples/directx9_example/main.cpp
)
target_link_libraries(imgui_test d3d9.lib)

set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )

message(STATUS "${MSVC_VERSION}=${${MSVC_VERSION}}")

if (${MSVC_VERSION} GREATER 1200)
   SET ( CMAKE_SHARED_LINKER_FLAGS /MANIFEST:NO )
endif()
