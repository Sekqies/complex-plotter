#include <graphics/3d/mesh.h>

void link_mesh(Mesh& mesh, const vector<float>& vertices, const vector<unsigned int>& indices) {
	glGenVertexArrays(1, &mesh.VAO);
	glGenBuffers(1, &mesh.VBO);
	glGenBuffers(1, &mesh.EBO);

	glBindVertexArray(mesh.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	mesh.index_count = (unsigned int)indices.size();
}
/// <summary>
/// Generates a mesh object of an n x n grid
/// </summary>
/// <param name="grid_size">The size of the grid (used for grid_size * grid_size)</param>
/// <returns>A grid mesh</returns>
Mesh generate_grid_mesh(const int grid_size) {
	vector<float> vertices;
	vector<unsigned int> indices;

	for (unsigned int z = 0; z <= grid_size; ++z) {
		for (unsigned int x = 0; x <= grid_size; ++x) {
			float u = static_cast<float>(x) / grid_size;
			float v = static_cast<float>(z) / grid_size;

			vertices.push_back(u * 2.0f - 1.0f);
			// y is modified at runtime
			vertices.push_back(0.0f);
			vertices.push_back(v * 2.0f - 1.0f);
		}
	}
	for (unsigned int z = 0; z < grid_size; ++z) {
		for (int x = 0; x < grid_size; ++x) {
			int row1 = z * (grid_size + 1);
			int row2 = (z + 1) * (grid_size + 1);

			indices.push_back(row1 + x);
			indices.push_back(row2 + x);
			indices.push_back(row1 + x + 1);

			indices.push_back(row1 + x + 1);
			indices.push_back(row2 + x);
			indices.push_back(row2 + x + 1);
		}
	}
	Mesh mesh;
	link_mesh(mesh, vertices, indices);
	return mesh;
}

