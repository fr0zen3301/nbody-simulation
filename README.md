# EMMA - Engine for Multi-body Mechanics and Astrophysics 
EMMA is a computational framework for simulating and visualising gravitational N-body systems in real time. It models the dynamics of interacting celestial bodies under Newtonian gravity, enabling the exploration of orbital motion, gravitational interactions, and emergent multi-body behaviour.

The engine is implemented in C++ with SFML ([the tutorials and documentation.](https://www.sfml-dev.org/learn/))and provides configurable physical parameters, real time visualisation, and trajectory tracking.

Built with [SFML 3](https://www.sfml-dev.org/learn/) for windowing, input, and rendering.

## What it does

Every body attracts every other body. The force on each pair comes straight from Newton:

```
F = G * m1 * m2 / r^2
```

That force is integrated forward in time to move the bodies. The simulation starts with a small three-body system and you grow it from there.

### Features

- **Velocity Verlet integration.** Positions and velocities advance with a symplectic integrator instead of plain Euler. This matters more than it sounds: Euler quietly leaks energy every step, so stable orbits slowly spiral out or collapse. Verlet uses the average of the acceleration at the start and end of each step, which keeps the total energy nearly flat over long runs.
- **Softening.** Up close, `1/r^2` blows up toward infinity and the simulation explodes. A softening term in the denominator (`r^2 + e^2`) smooths the force at short range so close passes behave instead of detonating.
- **Collision merging.** When two bodies get within a set distance they merge into one. The combined body keeps the total mass and a velocity that conserves momentum, positioned at the center of mass of the two.
- **Color by mass.** Each body is colored by how heavy it is (see the table below).
- **Motion trails.** Every body leaves a fading path behind it, capped at 500 points so memory stays bounded.
- **Starfield background.** A thousand static stars sit behind the action for depth.
- **Total energy readout.** The simulation can report kinetic plus potential energy. A conserved quantity is the best debugging tool there is: if the number drifts, the physics is wrong.

### Controls

- **Left click** drops a new body at the cursor.
- **Scroll / trackpad** zooms toward the cursor, with a bit of damping so it does not feel twitchy.
- **Resize the window** to reveal more space. The scale stays fixed, so a bigger window shows more of the universe rather than stretching what is already there.

## Color legend

Bodies are colored by mass:

| Color | Mass |
|-------|------|
| Red | greater than 8e6 |
| Orange | greater than 5e6 |
| Yellow | greater than 2e6 |
| Cyan | greater than 1e5 |
| White | anything smaller (moons, planets) |

## Building

You need SFML 3 installed. On macOS with Homebrew:

```
brew install sfml
```

Then compile:

```
clang++ -std=c++17 \
  -I/opt/homebrew/include \
  -L/opt/homebrew/lib \
  main.cpp Simulation.cpp Body.cpp \
  -lsfml-graphics -lsfml-window -lsfml-system \
  -o nbody
```

Run it:

```
./nbody
```

If your Homebrew prefix is different (Intel Macs use `/usr/local`), adjust the include and library paths to match.

## Project structure

| File | Role |
|------|------|
| `main.cpp` | Window setup, the event loop, input handling, zoom, and rendering. |
| `Simulation.hpp` / `Simulation.cpp` | Holds all the bodies, runs one physics step (force calculation, integration, merging), and draws everything. |
| `Body.hpp` / `Body.cpp` | A single body: mass, position, velocity, force, and the gravitational force calculation against another body. |
| `Vector2D.hpp` | Header-only 2D vector with the math operators the rest of the code leans on. |

## How a step works

Each frame, `Simulation::update(dt)` runs the Velocity Verlet cycle:

1. **Drift.** Move every position forward using current velocity and stored acceleration.
2. **Recompute forces** at the new positions. This is the O(N^2) part: every pair of bodies is checked once, and Newton's third law lets each pair share one force calculation (`+F` on one body, `-F` on the other).
3. **Kick.** Update every velocity using the average of the old and new acceleration, then store the new acceleration for the next frame.
4. **Merge** any bodies that have collided.

## Roadmap

- **Barnes-Hut quad-tree** to bring force computation from O(N^2) down to O(N log N). The current pairwise loop is fine for a handful of bodies but chokes past a few hundred. A quad-tree groups distant bodies and treats each clump as a single point mass, which is how large simulations stay fast.
- **On-screen energy display** using an SFML font, so the energy readout is visible during a run instead of printed to the console.
- **Preset systems** to load known configurations: the figure-eight three-body orbit, Lagrange points, a toy solar system.
- **3D mode**, extending the vector math to three dimensions and switching to a 3D camera.
- **Adjustable spawn parameters** so a click can set mass, position, and initial velocity instead of using fixed defaults.
