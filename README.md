# Mesher: Fast 3D mesh viewer with OpenGL and SDL2

## Installation
Make sure you have the following installed
- `ASSIMP`
- `SDL2`
- `OpenGL 4.6` 

```sh
git clone https://github.com/mohamedrezk122/mesher
cd mesher
mkdir build
cd build
cmake ..
make -j 4 
``` 

## Usage
you can pass a file through CLI
```
./mesher bunny.stl
```
or
```
./mesher
```
and then drag and drop a file to the window

## TODO
- [ ] draw a grid bed under the mesh
- [ ] fix the initial camera position
- [ ] add some labels to window (e.g. filename, num of traingles/faces)
- [ ] adapt to lower versions of OpenGL