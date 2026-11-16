# Experiment 6: 3D Sierpinski Gasket

## Aim

To represent a tetrahedron using four 3D vertices and recursively subdivide it to generate and interactively display a 3D Sierpinski gasket.

## Theory

A tetrahedron has four triangular faces. For every recursive step, the midpoint of each of its six edges is calculated. Those midpoints define four half-size tetrahedra at the original corners. The central inverted tetrahedron is omitted, producing the gaps characteristic of the Sierpinski gasket.

At depth `n`, the number of terminal tetrahedra is `4^n`. The implementation stops at depth zero and draws the four faces. It caps input at depth 6 because the rendering work increases exponentially.

## Algorithm

1. Read and validate a recursion depth from 0 through 6.
2. Define the four vertices of the initial tetrahedron.
3. In `subdivide(a,b,c,d,depth)`, draw the tetrahedron if `depth == 0`.
4. Otherwise calculate midpoints `ab`, `ac`, `ad`, `bc`, `bd`, and `cd`.
5. Recursively subdivide the four corner tetrahedra using `depth - 1`.
6. Apply view rotations and render with depth testing.

## Tests and expected observations

| Depth | Tetrahedra | Observation |
|---:|---:|---|
| 0 | 1 | One complete tetrahedron |
| 1 | 4 | Four corner tetrahedra and one central gap |
| 2 | 16 | Each corner tetrahedron has the same pattern |
| 3 | 64 | Clear self-similar 3D gasket |

Invalid text, negative values, and values over 6 are rejected and prompted again. Rotation and wireframe mode make the internal gaps easy to inspect.

## Review answers

1. Four smaller corner tetrahedra are generated from every tetrahedron at each level.
2. Omitting the central tetrahedron creates the empty space and self-similar fractal structure of the Sierpinski gasket.
3. The base case is recursion depth zero; at that point the tetrahedron is rendered instead of subdivided again.

# Experiment 7: Menu-driven Animated Flag with Bezier Curves

## Aim

To implement cubic Bezier curves, use them as the upper and lower boundaries of a flag, animate their control points, and control the result through a menu.

## Theory

A cubic Bezier curve uses four control points and is evaluated for `0 <= t <= 1`:

`B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t)t^2 P2 + t^3 P3`

`P0` and `P3` are the endpoints. `P1` and `P2` pull the curve and determine its interior direction and curvature. The program samples corresponding points on an upper and lower Bezier curve and joins them as a quad strip. The left endpoints remain fixed at the pole; the other control points vary smoothly with sine functions.

## Algorithm

1. Define four control points for both the upper and lower flag boundaries.
2. Sample each curve from `t = 0` to `t = 1` using Bernstein weights.
3. Join corresponding upper/lower samples to form the flag surface.
4. Every 16 ms, advance a phase by an amount proportional to wind speed.
5. Recalculate moving control points from the phase and redraw.
6. Use the right-click menu to start, stop, reset, change wind speed, change colour, or exit.

## Tests and expected observations

- Start: phase advances and the flag waves.
- Stop: the current animation state freezes.
- Reset: phase, speed, colour, and running state return to defaults.
- Wind speed: 0.5x, 1.0x, and 2.0x visibly change wave rate.
- Colour: blue, red, and green apply immediately.
- Exit: the window closes cleanly.
- At all phases, the left edge remains fixed to the pole.

For evidence, capture screenshots after stopping the animation at three visibly different phases and open the right-click menu for a fourth screenshot showing all required options.

## Review answers

1. `P0` determines the start and `P3` determines the end of a cubic Bezier curve.
2. Moving `P1` and `P2` changes the curve's tangents and curvature without changing its endpoints.
3. Use small timer increments, smooth functions such as sine waves, stable frame timing, and continuous control-point motion. Adjacent Bezier segments should also share endpoints and compatible tangents when multiple segments are used.

