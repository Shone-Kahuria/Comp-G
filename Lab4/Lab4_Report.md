# Lab 4 OpenGL Area Filling Algorithms

This report documents a single C++ implementation of the required OpenGL and pixel-based area-filling exercises. `AreaFill.cpp` contains all algorithms, `screenshots/` contains the output evidence, and `results.txt` contains the recorded benchmark data. The implementation completes Assignments 12-19 and answers Questions 20-26.

## Build / run
- Interactive (needs freeglut): `cl /EHsc AreaFill.cpp freeglut.lib opengl32.lib glu32.lib` then `AreaFillGL.exe` (`freeglut.dll` is included in this folder). `AreaFillGL.exe --shots` saves the pentagon modes and an additional OpenGL triangle screenshot.
- Keys: `0` GL_POLYGON, `1` Boundary 4, `2` Boundary 8, `3` Flood, `4` Scan-line, `t`/`p` triangle/pentagon, `c` cycle RGB colour, `b` benchmark and BMP export, `Esc` quit.
- Headless (no GL needed): `cl /O2 /EHsc /DHEADLESS AreaFill.cpp && AreaFill.exe`

Assignment mapping: 12/13 = `GL_POLYGON` with the `t` and `p` keys; 14 = `boundaryFill(...,false)`; 15 = `boundaryFill(...,true)`; 16 = `floodFill`; 17 = `scanLineFill`; 18 = the `c` key and `PALETTE`; 19 = `benchmark()` and `results.txt`.

## Algorithm explanations
- **Boundary Fill**: from a seed, colour every pixel that is neither boundary nor fill colour, spreading to 4 (or 8) neighbours.
- **Flood Fill**: from a seed, replace every connected pixel of the old colour with the new colour.
- **Scan-Line**: for each y between min and max, intersect the scan line with edges (half-open `[ymin,ymax)`, horizontals skipped), sort the x values, and fill between pairs using the even-odd rule.
- Implementation note: the pixel fills use an explicit stack, not recursion, because the recursive pseudocode overflows the call stack on ~80,000-pixel regions. Boundaries are drawn with a 4-connected Bresenham line so the 8-connected fill does not leak through diagonal gaps.

## Comparison (600x600 buffer, average of 20 runs, -O2)

| Shape | Algorithm | Pixels | ms |
|---|---|---|---|
| Triangle | Boundary 4 | 69,054 | 2.66 |
| Triangle | Boundary 8 | 69,054 | 6.68 |
| Triangle | Flood 4 | 69,054 | 2.67 |
| Triangle | Scan-line | 70,150 | 0.16 |
| Pentagon | Boundary 4 | 83,642 | 3.64 |
| Pentagon | Boundary 8 | 83,642 | 7.71 |
| Pentagon | Flood 4 | 83,642 | 3.40 |
| Pentagon | Scan-line | 85,080 | 0.18 |

Times include clearing the buffer and drawing the outline. Against Boundary 4 and Flood Fill in this recorded run, Scan-Line is about 15-20x faster because it fills ordered spans without neighbour tests or a work stack. It also paints the outline pixels, which explains its slightly higher pixel count. Boundary 8 is about 2x slower than Boundary 4 because it examines eight neighbours per filled pixel. Boundary Fill and Flood Fill perform similarly on a single-colour region. Scan-Line needs the polygon vertices, while the seed-based fills operate from pixel colours.

| Algorithm | Needs | Best for |
|---|---|---|
| Boundary Fill | seed + closed boundary colour | closed shapes |
| Flood Fill | seed + old colour | paint-bucket tools |
| Scan-Line | polygon vertices | polygon rendering |
| GL_POLYGON | vertices (GPU) | basic polygon drawing (convex only) |

## Discussion answers
**20. Boundary Fill vs Flood Fill:** Boundary Fill stops at a specified boundary colour and can fill a region containing different interior colours. Flood Fill replaces only pixels connected to the seed that match a specified old colour.

**21. 4-connected vs 8-connected filling:** A 4-connected fill moves up, down, left, and right. An 8-connected fill also moves diagonally. The 8-connected version needs a 4-connected boundary; otherwise it can escape through diagonal gaps.

**22. Closed boundary:** Any gap can let Boundary Fill escape into the background because the boundary-colour stop condition is no longer continuous.

**23. Multi-colour Flood Fill:** Flood Fill changes only the connected pixels matching the seed's original colour. Other colours stop the fill, so the result can be partial, as shown in `flood_multicolour.png`.

**24. Scan-Line pixel selection:** For each row, the algorithm finds and sorts edge intersections, then fills between pairs 1-2, 3-4, and so on. This is the even-odd rule.

**25. glBegin(GL_POLYGON) and glEnd():** These calls delimit the vertex list for one filled convex polygon in the legacy immediate-mode API. The rasterizer fills the polygon interior. `GL_POLYGON` is unavailable in the modern core profile.

**26. GL_LINE_LOOP vs GL_POLYGON:** `GL_LINE_LOOP` draws only the closed outline. `GL_POLYGON` fills the interior and requires a convex polygon for reliable results.
