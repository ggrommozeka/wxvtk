
# Software Development Task

This project integrates **wxWidgets 3.0.3** and **VTK 8.0** to create an interactive graphical interface for mesh manipulation.

## Prerequisites
- **Visual Studio 2022 (x64)** 
- **wxWidgets 3.0.3** [link](https://github.com/wxWidgets/wxWidgets/releases/tag/v3.0.3)
- **VTK 8.0** [link](https://gitlab.kitware.com/vtk/vtk/tree/v8.0.0)

## Building wxWidgets with nmake
To build wxWidgets 3.0.3 with **nmake**, follow these steps:
1. Open a **Developer Command Prompt for Visual Studio 2022**.
2. Navigate to the wxWidgets directory:
   ```cmd
   cd Path_to_wxWidgets
   ```
3. Build wxWidgets with the following command:
   ```cmd
   nmake -f makefile.vc BUILD=debug SHARED=1 TARGET_CPU=X64
   ```
   To build a release version, use:
   ```cmd
   nmake -f makefile.vc BUILD=release SHARED=1 TARGET_CPU=X64
   ```

## Building VTK
Ensure that VTK 8.0 is built in **Release** mode for optimal performance. You can follow the official instructions available at [VTK/Configure and Build](https://vtk.org/Wiki/VTK/Configure_and_Build).

## Setting Environment Variables
1. **Set `VTK_DIR`:** Point to the directory containing the installed VTK 8.0 files.
2. **Update `Path`:** Add the following directories to your system's `Path` environment variable:
   - `%VTK_DIR%\bin`
   - `Path_to_wxWidgets\lib\vc_x64_dll`

## Preparation Before Build
1. **Update `CMakeLists.txt`:**
   Replace the following paths with the actual paths on your system:
   ```cmake
   set(wxWidgets_ROOT_DIR "E:/VTK_projects/src/wxWidgets-3.0.3")
   set(wxWidgets_LIB_DIR "E:/VTK_projects/src/wxWidgets-3.0.3/lib/vc_x64_dll")
   set(wxWidgets_INCLUDE_DIR "E:/VTK_projects/src/wxWidgets-3.0.3/include")
   ```

   If you use a **release build** of wxWidgets, replace:
   ```cmake
   set(wxWidgets_CONFIGURATION "mswud")
   ```
   with:
   ```cmake
   set(wxWidgets_CONFIGURATION "mswu")
   ```

## Build Instructions for the Project
1. Open **Visual Studio 2022**.
2. Open the project folder.
3. Configure and generate build files:
   - Ensure the target platform is **x64**.
   - Verify that CMake detects the updated paths for wxWidgets and VTK.
   - Use `CMakeLists.txt` to build project
4. Build the project:
   - Click **Build > Build Solution** or press `Ctrl+Shift+B`.

## Usage
1. Launch the application.
2. Use the **File** menu to load a 3D mesh.
3. Manipulate the mesh using available tools such as lasso selection and edge toggling.
4. Save the selected sub-mesh using the **Save Sub-Mesh** button as .obj or .stl files.

