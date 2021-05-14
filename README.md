# Spherical Harmonics Exponentiation

Implementation of *Real-time Soft Shadows in Dynamic Scenes using Spherical Harmonic Exponentiation*. Supports static scene of diffuse objects and area / environment light.

### Build & Run

Linux dependencies for glfw: xorg-dev libgl1-mesa-dev

build

```
mkdir build && cd build
cmake ..
make
```

run interactive

```
./playground
```

output single image then quit

```
./playground -s
```

To test at different SH order, edit `shorder.hpp`

To change scene configuration / SHEXP algorithm, edit `main.cpp`

### Preprocess tools

`util/envmap` project θ-φ parameterized light probe image onto SH bases.

`util/makelut` precompute shlog of visibility functions & corresponding shexp OL coefficients.

`gamma/src/makesparse` tabulate SH tripling tensor

`github.com/111116/sphere-set-approximation` generate sphere representation of object.



