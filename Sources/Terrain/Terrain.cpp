#include "Terrain.h"

// Default
Terrain::Terrain() : m_gridSize(0), m_grid(nullptr)
{
	m_gridMinIndex = vec2i(-std::numeric_limits<int>::max());
}
Terrain::~Terrain()
{
	m_areas.clear();
	RecomputeGrid();

	for (auto& clipmap : m_clipmaps)
	{
		glDeleteVertexArrays(1, &clipmap.m_VAO);
		glDeleteBuffers(1, &clipmap.m_vertexbuffer);
		glDeleteBuffers(1, &clipmap.m_arraybuffer);
	}
	m_clipmaps.clear();
}
//

// public functions
void Terrain::initializeClipmaps()
{
	const int lodFaceCount[] = { 256, 128, 64, 32, 16, 8, 4, 2 };
	constexpr int lodCount = sizeof(lodFaceCount) / sizeof(int);

	std::vector<vec4f> vertices;
	std::vector<uint16_t> indices;
	vertices.reserve((lodFaceCount[0] + 1) * (lodFaceCount[0] + 1));
	indices.reserve(lodFaceCount[0] * lodFaceCount[0] * 6);

	m_clipmaps.clear();
	m_clipmaps.resize(lodCount);

	for (int lod = 0; lod < lodCount; lod++)
	{
		// allocate
		int faceCount = lodFaceCount[lod];
		vec2f faceSize = vec2f(1.f / faceCount);
		vertices.clear();
		indices.clear();

		// push vertices
		for (int i = 0; i <= faceCount; i++)
			for (int j = 0; j <= faceCount; j++)
				vertices.push_back(vec4f(i * faceSize.x -0.5f, 0.f, i * faceSize.y - 0.5f, 1.f));

		// push indices
		for (int i = 0; i < faceCount; i++)
			for (int j = 0; j < faceCount; j++)
			{
				uint16_t i0 = i * (faceCount + 1) + j;
				uint16_t i1 = (i + 1) * (faceCount + 1) + j;
				uint16_t i2 = (i + 1) * (faceCount + 1) + j + 1;
				uint16_t i3 = i * (faceCount + 1) + j + 1;

				bool evenFaceIndex = ((i + j) & 0x01) == 0;
				if (evenFaceIndex)
				{
					indices.push_back(i0);
					indices.push_back(i1);
					indices.push_back(i2);
					indices.push_back(i0);
					indices.push_back(i2);
					indices.push_back(i3);
				}
				else
				{
					indices.push_back(i0);
					indices.push_back(i1);
					indices.push_back(i3);
					indices.push_back(i1);
					indices.push_back(i2);
					indices.push_back(i3);
				}
			}

		//	initialize VBO
		glGenBuffers(1, &m_clipmaps[lod].m_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_clipmaps[lod].m_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec4f), vertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &m_clipmaps[lod].m_arraybuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_clipmaps[lod].m_arraybuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

		//	initialize VAO
		glGenVertexArrays(1, &m_clipmaps[lod].m_VAO);
		glBindVertexArray(m_clipmaps[lod].m_VAO);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, m_clipmaps[lod].m_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_clipmaps[lod].m_arraybuffer);
		glBindVertexArray(0);
	}
}


void Terrain::AddArea(vec2i index, TerrainArea& area, bool recomputeIfNeeded)
{
	vec2i_ordered key(index);
	if (m_areas.find(key) != m_areas.end())
		return;

	auto it = m_areas.insert({ key, area });

	vec2i gridindex = index - m_gridMinIndex;
	if (index.x >= 0 && index.x < m_gridSize.x && index.y >= 0 && index.y < m_gridSize.y)
		m_grid[gridindex.x][gridindex.y] = &it.first->second;
	else if (recomputeIfNeeded)
		RecomputeGrid();
}
void Terrain::RemoveArea(vec2i index)
{

}
//

// protected functions
void Terrain::RecomputeGrid()
{
	// dele grid
	for (int i = 0; i < m_gridSize.x; i++)
		delete[] m_grid[i];
	delete[] m_grid;
	m_grid = nullptr;
	m_gridSize = vec2i(0, 0);
	m_gridMinIndex = vec2i(-std::numeric_limits<int>::max());

	// create a new one
	if (!m_areas.empty())
	{
		vec2i gridMax = -m_gridMinIndex;
		for (auto& it : m_areas)
		{
			m_gridMinIndex = vec2i::min(m_gridMinIndex, it.first.v);
			gridMax = vec2i::max(gridMax, it.first.v);
		}
		m_gridSize = gridMax - m_gridMinIndex + vec2i(1);

		m_grid = new TerrainArea**[m_gridSize.x];
		for (int i = 0; i < m_gridSize.x; i++)
		{
			m_grid[i] = new TerrainArea*[m_gridSize.y];
			for (int j = 0; j < m_gridSize.y; j++)
				m_grid[i][j] = nullptr;
		}

		for (auto& it : m_areas)
		{
			vec2i gridindex = it.first.v - m_gridMinIndex;
			it.second.m_gridIndex = gridindex;
			m_grid[gridindex.x][gridindex.y] = &it.second;
		}
	}
}
//