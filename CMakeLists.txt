cmake_minimum_required(VERSION 3.5)
project(wxVTKRenderWindowInteractor)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_USE_RELATIVE_PATHS ON)
if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /subsystem:windows")
endif()

# Include the include directory for header files
include_directories(include)

# Find and include VTK package
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Set wxWidgets configuration and paths
set(wxWidgets_CONFIGURATION "mswud")
set(wxWidgets_ROOT_DIR "E:/VTK_projects/src/wxWidgets-3.0.3")
set(wxWidgets_LIB_DIR "E:/VTK_projects/src/wxWidgets-3.0.3/lib/vc_x64_dll")
set(wxWidgets_INCLUDE_DIR "E:/VTK_projects/src/wxWidgets-3.0.3/include")

# Find and include wxWidgets package
find_package(wxWidgets REQUIRED core base gl)
include("${wxWidgets_USE_FILE}")

# Add wxVTKRenderWindowInteractor library
add_library(wxVTKRenderWindowInteractor SHARED src/wxVTKRenderWindowInteractor.cxx)
target_include_directories(wxVTKRenderWindowInteractor PUBLIC include)
target_link_libraries(wxVTKRenderWindowInteractor ${VTK_LIBRARIES} ${wxWidgets_LIBRARIES})

# Add project executable
add_executable(main
  src/main.cpp
  src/MyFrame.cpp
  src/MyInteractorStyle.cpp
  src/MyLassoInteractorStyle.cpp
)

# Set target properties
set_target_properties(main PROPERTIES WIN32_EXECUTABLE TRUE)
target_include_directories(main PRIVATE include)
target_compile_definitions(main PUBLIC WXUSINGDLL)

# Link libraries to main
target_link_libraries(main
  wxVTKRenderWindowInteractor
  ${VTK_LIBRARIES}
  ${wxWidgets_LIBRARIES}
)