# LITTLE WORLD · Cosmic Sandbox

**A pocket universe in motion.** An atmospheric, browser-based cosmic sandbox where you can explore a star system, select planets, and control the passage of simulation time.

> First milestone: a polished interactive 3D star system. Future milestones add real gravitational integration, world generation, persistent seeds, and emergent planetary systems.

## Run it

This is a static web application using Three.js as an ES module. No local build tool or package installation is required. Open the deployed GitHub Pages URL after the Actions workflow succeeds. For local development, serve the repository root with any static HTTP server; ES modules generally should not be opened from a `file://` URL.

## Controls

- Drag the scene to rotate the camera.
- Scroll to zoom.
- Select a planet using the right-hand planet list or click the planet in the scene.
- Focus the selected planet with the target button.
- Pause/resume the simulation.
- Adjust simulation time to ¼×, 1×, 4×, or 12×.
- Reset the orbital positions.

## Architecture

- `index.html` — semantic application shell and UI.
- `styles.css` — responsive atmospheric interface.
- `app.js` — Three.js scene, deterministic starfield, celestial body definitions, rendering loop, camera interaction, and simulation controls.
- `.github/workflows/pages.yml` — automated GitHub Pages deployment.

### Design principles

1. **Simulation before spectacle.** Visual polish must be backed by clear, inspectable rules.
2. **Reproducibility.** Generated worlds will use explicit seeds so the same seed can recreate a system.
3. **Progressive complexity.** Keep the first version fast and dependency-light; isolate orbital mechanics, generation, persistence, and UI as the project grows.
4. **Cloud-first development.** GitHub is the source of truth. GitHub Actions validates and deploys the app so development can continue without a local laptop.

## Roadmap

### Phase 1 — First light
- [x] Atmospheric 3D star system
- [x] Planet selection and inspection summary
- [x] Camera orbit/zoom
- [x] Simulation pause and time-speed controls
- [x] GitHub Pages deployment workflow
- [ ] Accessibility and mobile interaction pass

### Phase 2 — Orbital mechanics
- [ ] Separate simulation engine from renderer
- [ ] Implement reproducible Newtonian two-body orbital elements
- [ ] Display orbital period, semi-major axis, eccentricity, and velocity
- [ ] Unit tests for orbital state and time-step invariants

### Phase 3 — World generation
- [ ] Seeded star-system generator
- [ ] Planet composition, temperature, atmosphere, and moon generation
- [ ] Save/load world seeds locally
- [ ] System discovery log

### Phase 4 — Universe laboratory
- [ ] N-body gravity experiment mode with bounded time steps
- [ ] Trails, time scrubbing, event timeline
- [ ] Stable/unstable orbit diagnostics
- [ ] Exportable world cards and shareable seeds

### Phase 5 — Flagship polish
- [ ] Procedural planetary textures and shader-based atmospheres
- [ ] Reduced-motion mode and low-performance rendering profile
- [ ] Performance profiling and browser compatibility checks
- [ ] Public demo and project write-up

## Technical note

The initial build is a visual prototype: the bodies move on preset circular paths. It is **not yet a physically accurate N-body simulation**. The next engineering milestone is to introduce a tested simulation engine before describing the sandbox as a physics simulator.

## License

MIT — see `LICENSE`.
