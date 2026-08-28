# EMMA - Engine for Multi-body Mechanics and Astrophysics 
EMMA is a computational framework for simulating and visualising gravitational N-body systems in real time. It models the dynamics of interacting celestial bodies under Newtonian gravity, enabling the exploration of orbital motion, gravitational interactions, and emergent multi-body behaviour.

The engine is implemented in C++ with SFML ([tutorials and documentation](https://www.sfml-dev.org/learn/)) and provides configurable physical parameters, real time visualisation, and trajectory tracking.

Built with [SFML 3](https://www.sfml-dev.org/learn/) for windowing, input, and rendering, and [Dear ImGui](https://github.com/ocornut/imgui) for the control panel.

## What it does

Every body attracts every other body. The force on each pair comes straight from Newton:

```
F = G * m1 * m2 / r^2
```

That force is integrated forward in time to move the bodies. The simulation starts with a small three-body system and you grow it from there.

### Features

- **Velocity Verlet integration.** Positions and velocities advance with a symplectic integrator instead of plain Euler. This matters more than it sounds: Euler quietly leaks energy every step, so stable orbits slowly spiral out or collapse. Verlet uses the average of the acceleration at the start and end of each step, which keeps the total energy nearly flat over long runs. The physics runs on a fixed timestep decoupled from the framerate, so the simulation behaves identically at 30 FPS and 144 FPS.
- **Softening.** Up close, `1/r^2` blows up toward infinity and the simulation explodes. A softening term in the denominator (`r^2 + e^2`) smooths the force at short range so close passes behave instead of detonating. The energy readout uses the matching softened potential, so the conserved quantity stays honest.
- **Collision merging.** When two bodies touch (their visual radii overlap) they merge into one. The combined body keeps the total mass and a velocity that conserves momentum, positioned at the center of mass of the two. A brief expanding ring marks the merge.
- **Mass-scaled rendering.** Body radius grows logarithmically with mass, so a glance at the screen shows the hierarchy of the system.
- **Color by mass.** Each body is colored by how heavy it is, following real stellar classification (see the table below).
- **Spawn controls.** An ImGui panel sets mass, speed, and direction for new bodies, with one-click spawning at the view center.
- **Motion trails.** Every body leaves a fading path behind it, capped at 500 points so memory stays bounded.
- **Starfield background.** A thousand static stars with varied size, brightness, and tint sit behind the action for depth.
- **Total energy readout.** The HUD shows kinetic plus potential energy live. A conserved quantity is the best debugging tool there is: if the number drifts, the physics is wrong.

### Controls

- **Left click** drops a new body at the cursor.
- **Spawn Controls panel** sets the mass, speed, and direction of spawned bodies.
- **Scroll / trackpad** zooms toward the cursor, with a bit of damping so it does not feel twitchy.
- **Resize the window** to reveal more space. The scale stays fixed, so a bigger window shows more of the universe rather than stretching what is already there.

## Color legend

Bodies are colored by mass:

| Color | Mass | Star type |
|-------|------|-----------|
| 🔵 Blue-white | greater than 8e6 | massive giant |
| ⚪ White | greater than 5e6 | hot star |
| 🟡 Yellow | greater than 2e6 | sun-like |
| 🟠 Orange | greater than 1e5 | orange dwarf |
| 🔴 Red | anything smaller | red dwarf |

Colors follow real stellar classification: the most massive stars burn hottest and bluest, while the smallest glow dim red.

## Building

You need SFML 3 installed. On macOS with Homebrew:

```
brew install sfml
```

Then compile:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run it from the project root (the font is loaded by relative path):

```
./build/nbody
```

The first build downloads and compiles ImGui automatically (via CMake FetchContent), so it needs an internet connection and takes a minute. Later builds are fast.

## Project structure

| File | Role |
|------|------|
| `main.cpp` | Window setup, the event loop, input handling, zoom, the ImGui panel, and rendering. |
| `Simulation.hpp` / `Simulation.cpp` | Holds all the bodies, runs one physics step (force calculation, integration, merging), and draws everything. |
| `Body.hpp` / `Body.cpp` | A single body: mass, position, velocity, force, and the gravitational force calculation against another body. |
| `Vector2D.hpp` | Header-only 2D vector with the math operators the rest of the code leans on. |
| `CMakeLists.txt` | The build recipe: compiles the sources, finds SFML, and fetches ImGui. |

## How a step works

Each frame, `Simulation::update(dt)` runs the Velocity Verlet cycle:

1. **Drift.** Move every position forward using current velocity and stored acceleration.
2. **Recompute forces** at the new positions. This is the O(N^2) part: every pair of bodies is checked once, and Newton's third law lets each pair share one force calculation (`+F` on one body, `-F` on the other).
3. **Kick.** Update every velocity using the average of the old and new acceleration, then store the new acceleration for the next frame.
4. **Merge** any bodies that have collided.

## Roadmap

- **Barnes-Hut quad-tree** to bring force computation from O(N^2) down to O(N log N). The current pairwise loop is fine for a handful of bodies but chokes past a few hundred. A quad-tree groups distant bodies and treats each clump as a single point mass, which is how large simulations stay fast.
- **Preset systems** to load known configurations: the figure-eight three-body orbit, Lagrange points, a toy solar system with elliptical orbits.
- **3D mode**, extending the vector math to three dimensions and switching to a 3D camera.