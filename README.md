# SpaceTime

Real-time n-body simulator, built in C++ with OpenGL.

https://github.com/user-attachments/assets/55c939fa-84de-4002-ab8c-6647669905db

---

## Features

- Gravity via compute shaders (SSBO)
- 4th-order Suzuki–Yoshida symplectic integration
- Real-time gravity well visualization

---

## Build

```bash
cd spacetime
./vcpkg/bootstrap-vcpkg.sh
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
