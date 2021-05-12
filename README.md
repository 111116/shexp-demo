# Spherical Harmonics Playground

Implementation of *Real-time Soft Shadows in Dynamic Scenes using Spherical Harmonic Exponentiation*.



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


n=6: ~6fps
n=7: ~2.7fps
n=8: doesn't load

