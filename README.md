# Bubbles

**A physically-based bubble rendering and animation system implemented in C/C++ with OpenGL.**

This project simulates the visual behavior of soap bubbles, capturing the optical phenomena that make them visually distinctive: thin-film interference (the rainbow shimmer), refraction, and specular reflection on curved surfaces. Built as a team project for a Computer Graphics course.

---

## 🎯 What This Project Does

Real bubbles are surprisingly hard to render correctly. Their thin soap film bends light, splits it into colors, and reflects the environment in distorted ways — all happening simultaneously on a moving, deforming surface. We built a complete rendering pipeline that handles each of these effects in a way that runs at interactive frame rates, while still being physically grounded.

The system supports:

- **Triangle-mesh-based bubble surfaces**, loaded from OBJ files and processed into renderable geometry
- **Physically-based shading** with Fresnel reflectance, refraction, and thin-film interference computed per-pixel
- **Environment mapping** via cube-map textures, so bubbles reflect their surroundings
- **Real-time rendering pipeline** built on OpenGL with custom GLSL shaders
- **Ray-traced reference images** for verifying the rasterized look matches the underlying physics

---

## 🖼️ Project Structure

```
Bubbles/
├── CGL/                   # Computer Graphics Library (math, mesh, shader utilities)
├── src/                   # Main application code (rendering pipeline, scene setup)
├── mesh/                  # OBJ mesh files for bubble surfaces
├── textures/              # Cube-map and environment textures
├── proposal/              # Initial project proposal
├── milestone/             # Mid-project milestone deliverables
├── deliverable/           # Final report, video, and writeup
├── CMakeLists.txt         # Build configuration
└── copy_shaders.cmake     # Shader copying step in the build
```

---

## ⚙️ Building and Running

### Requirements

- A C++17-capable compiler (GCC, Clang, or MSVC)
- CMake 3.10+
- OpenGL 3.3+ drivers
- GLFW and GLEW (typically resolved by CMake on most systems)

### Build

```bash
mkdir build && cd build
cmake ..
make -j
```

### Run

From the build directory:

```bash
./bubbles
```

The executable expects `mesh/` and `textures/` resources to be present relative to its working directory; the CMake configuration handles copying shaders automatically via `copy_shaders.cmake`.

---

## 🔬 Technical Highlights

### Thin-film interference

Soap bubbles get their iridescent color because light reflects off both the outer and inner surface of the soap film, and the two reflected waves interfere depending on the film thickness and the wavelength of light. We compute the resulting color shift per fragment, using a wavelength-dependent path-length difference and the standard interference formula.

### Fresnel reflection and refraction

The mix of reflected and refracted light at any point on a bubble depends on the angle between the surface normal and the view direction (the Fresnel equations). At grazing angles bubbles look more mirror-like; head-on, they're more transparent. The shader captures this using Schlick's approximation balanced against a refracted environment lookup.

### Ray tracing for verification

In addition to the real-time GLSL rasterization pipeline, the system includes a ray-tracing path used during development to check that the real-time shaders produce visually consistent results with a slower but more physically faithful reference. This was particularly useful for debugging the interference and refraction logic, where small errors in shader code can be very hard to spot from a single frame.

---

## 👥 Contributions

This was a team project. My primary contributions:

- **Ray tracing and ray-rendering modules** — the offline reference path used to verify the real-time shading logic
- **Thin-film interference and refraction math** — derivations and shader implementations
- **Integration work** — connecting the rendering modules with the team's mesh and animation code

103 commits across the project lifecycle.

---

## 📚 References

- *Real-Time Rendering* (Akenine-Möller et al.) — Fresnel approximations, environment mapping
- *Physically Based Rendering: From Theory to Implementation* (Pharr, Jakob, Humphreys) — ray tracing fundamentals
- Berkeley CS Computer Graphics course materials — pipeline architecture and shader design conventions

---

## 📜 License

Educational project. Code is provided as-is for reference and learning.
