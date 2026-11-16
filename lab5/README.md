# Computer Graphics Experiments 6 and 7

This Visual Studio/CMake project contains two independent C++ OpenGL programs:

- `experiment6_sierpinski` - recursively subdivides a tetrahedron into a 3D Sierpinski gasket.
- `experiment7_bezier_flag` - animates a flag surface bounded by cubic Bezier curves and provides a right-click menu.

FreeGLUT 3.6.0 is downloaded and compiled automatically during the first CMake configuration. An internet connection is therefore required once. No manual include or linker configuration is needed.

## Run in Visual Studio

1. Open Visual Studio and select **Open a local folder**.
2. Select this `lab5` folder (the folder containing `CMakeLists.txt`).
3. Wait for CMake configuration to finish. The first configuration downloads freeglut.
4. In the startup-item list on the toolbar, select `experiment6_sierpinski.exe` or `experiment7_bezier_flag.exe`.
5. Press **F5** (debug) or **Ctrl+F5** (run without debugging).

If Visual Studio says that C++ tools are missing, open Visual Studio Installer and add the **Desktop development with C++** workload, including CMake tools for Windows.

## Controls

### Experiment 6

- Enter a recursion depth from 0 to 6 in the console before the window opens.
- `+` / `-`: increase/decrease depth.
- Arrow keys or left-mouse drag: rotate the gasket.
- `W`: toggle solid/wireframe rendering.
- `R`: reset view and depth.
- `Esc`: exit.

Depths 0, 1, 2 and 3 render 1, 4, 16 and 64 tetrahedra respectively. Depth is capped at 6 (4096 tetrahedra) to keep interaction responsive.

### Experiment 7

- Right-click the window for **Start**, **Stop**, **Reset**, **Change Wind Speed**, **Change Colour**, and **Exit**.
- `Space`: start/stop animation.
- `R`: reset animation, colour and speed.
- `Esc`: exit.

## Command-line build (optional)

From a Visual Studio Developer PowerShell:

```powershell
cmake --preset visual-studio
cmake --build --preset debug
```

The executables will be in `build\Debug`.

See `REPORT_NOTES.md` for the theory, algorithms, test plan, and answers to all review questions.

