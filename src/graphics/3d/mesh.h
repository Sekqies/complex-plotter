#include <vector>
#include <glad/glad.h>

struct Mesh {
	unsigned int VAO, VBO, EBO;
	unsigned int index_count;
};

using std::vector;

/// <summary>
/// Generates a mesh object of an n x n grid
/// </summary>
/// <param name="grid_size">The size of the grid (used for grid_size * grid_size)</param>
/// <returns>A grid mesh</returns>
Mesh generate_grid_mesh(const int grid_size);

