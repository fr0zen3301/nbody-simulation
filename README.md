# EMMA - Engine for Multi-body Mechanics and Astrophysics 
EMMA is a computational framework for simulating and visualising gravitational N-body systems in real time. It models the dynamics of interacting celestial bodies under Newtonian gravity, enabling the exploration of orbital motion, gravitational interactions, and emergent multi-body behaviour.

The engine is implemented in C++ with SFML ([the tutorials and documentation.](https://www.sfml-dev.org/learn/))and provides configurable physical parameters, real time visualisation, and trajectory tracking.

**Colors are based on the objects' mass:**

🟥 - Supermassive bodies (mass > 8e6)
🟧 - Giant stars (mass > 5e6)
🟨 - Medium mass (mass > 2e6)
🟦 - Lighter objects (mass > 1e5)
⬜️ - Tiny (moons or planets) 

**TO-DO:** Add an option to initiliaze the mass, position and initial velocity of bodies