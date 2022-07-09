# Map Generation

2D image processing to produce maps, with a 3D visualizer for getting a better sense of scale.
Pixels should be a real-world size so that all algorithms operate on the same scale.
Map Generation is for generating a world overview that can be used to inform other procedural generation at a finer level, eg, settlement layout, playable terrain meshes, etc...

# Building

It should be possible to just build with the following (Ubuntu):
```
git clone https://github.com/MattRickS/mapGen.git
cd mapgen
./build.sh
```

## External packages
- glm (submodule)
- SFML (cmake FetchContent)

## Dependencies
Requires the following packages to be first installed. The `build.sh` installs these.

- libfreetype-dev
- libopenal-dev
- libvorbis-dev
- libflac-dev


# Planned Operations

- Heightmap generation
- Vignette (to ensure edges are below sea level)
- Flood fill to sea level from edges (saltwater map)
- Coastal Erosion
    - Sand deposit map
- Hyrdaulic Erosion
    - Erosion map (rocky ground)
    - Deposit map (fertile ground)
    - Pooled Water map (lakes)
    - Water flow map (rivers)
- Normal map generation
- Sun map (Assuming N and S most angles while moving from E to W, how much light hits a pixel, normalized)
- Flora Generation (Freshwater access, fertile/rocky ground, sun map)

- Biome Detection
    - Mountain
    - Hills
    - Forest
    - Plains
    - Riverlands
    - Swamp
    - Coastal
- Feature Detection
    - Bay
    - Strait
    - Isthmus
    - Archipelago
    - Lagoon
    - Gulf
    - Island
    - Peninsula
    - Cape
    - Ford
- Settlement Detection (Most likely candidates for settlements to start, eg, ford, river mouth, etc...)

- Settlement interaction
    - Roads
    - Ship routes
- Name Generation
    - Features
    - Rivers
    - Lakes
    - Settlements
    - Forests
    - Mountains

## Heightmap Generation

Want to be able to view the heightmap and the 3D rendering at the same time, ideally while able to modify settings in realtime.

Dependencies:
- glad / glfw for openGL
- glm for maths
- ImGui for interactive UI
- stb_image for reading/writing textures

Code Requirements:
- core/camera.h
- core/mesh.h
- shaders/heightmap.vs
- ui/controllers/camera.h
- ui/controllers/heightmap.h
- ui/views/scene.h
- ui/views/texture.h

Can start with a basic Perlin noise and take a look at other algorithms.
Might be nice to have a "mountain range" generation and expand the land from it.

## Coastal Erosion

## Hydraulic Erosion

## Edge detection

Landmass, saltwater, features, biomes, rivers, and lakes will want to have their boundaries represented in a data format to allow interaction and computation. Will need a way of processing the image data to output path information.
