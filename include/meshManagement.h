#pragma once

struct Mesh
{
    unsigned int VAO {};
    unsigned int indiciesLength {};
};
namespace MeshTools
{
    Mesh generateGrid(int gridSize);
    Mesh makeCube();
    Mesh makeTetrahedron();
}