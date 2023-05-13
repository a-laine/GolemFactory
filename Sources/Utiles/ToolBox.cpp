#include "ToolBox.h"
#include <Resources/Mesh.h>

#include <fstream>
#include <cctype>
#include <sys/stat.h>


//	Public functions
std::string ToolBox::openAndCleanCStyleFile(std::string targetFile, std::string commentBlockEntry, std::string commentLineEntry)
{
	//	open file to parse
	std::string output;
	std::ifstream file(targetFile);
	if (!file.good()) return output;

	//	parsing parameters initialization
	unsigned int parsingCommentBlockIndex = 0;
	unsigned int parsingCommentLineIndex = 0;
	bool actuallyParsingCommentBlock = false;
	bool actuallyParsingCommentLine = false;
	char currentChar;

	//	parse file and remove all comment block and line
	while (file.get(currentChar))
	{
		if (actuallyParsingCommentBlock)
		{
			if (currentChar == commentBlockEntry[parsingCommentBlockIndex])
			{
				if (parsingCommentBlockIndex == 0)	//	end of parsing entry block backward so stop parsing block
				{
					//	reset comment entry parsing parameter
					parsingCommentBlockIndex = 0;
					parsingCommentLineIndex = 0;
					actuallyParsingCommentBlock = false;
					actuallyParsingCommentLine = false;
				}
				else parsingCommentBlockIndex--;	//	parsing backward for end of block detection
			}
			else parsingCommentBlockIndex = (int)commentBlockEntry.size() - 1;	//	parsing block fail so reset parameter
		}
		else if (actuallyParsingCommentLine)
		{
			if (currentChar == '\n' || currentChar == '\r')	//	end line char detected so stop parsing comment line ('\r' and / or '\n' depending on platform)
			{
				//	reset comment entry parsing parameter
				parsingCommentBlockIndex = 0;
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = false;
				actuallyParsingCommentLine = false;

				//	push the end line char to keep a cooherence if using getline on the string after
				output.push_back(currentChar);
			}
		}
		else if (!commentBlockEntry.empty() && currentChar == commentBlockEntry[parsingCommentBlockIndex])
		{
			if (parsingCommentBlockIndex >= commentBlockEntry.size() - 1)	//	comment block entry string match entirely -> begin parsing a comment line
			{
				//	erase last pushed char coresponding to the comment block entry string
				output.erase(std::prev(output.end(), parsingCommentBlockIndex), output.end());

				//	set parameters for parsing a block
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = true;
				actuallyParsingCommentLine = false;
				continue;
			}
			else parsingCommentBlockIndex++;	//	parsing forward

			if (!commentLineEntry.empty() && currentChar == commentLineEntry[parsingCommentLineIndex])
			{
				if (parsingCommentLineIndex >= commentLineEntry.size() - 1)	//	comment line entry string match entirely -> begin parsing a comment line
				{
					//	erase last pushed char coresponding to the comment line entry string
					output.erase(std::prev(output.end(), parsingCommentLineIndex), output.end());

					//	set parameters for parsing a line
					parsingCommentBlockIndex = 0;
					parsingCommentLineIndex = 0;
					actuallyParsingCommentBlock = false;
					actuallyParsingCommentLine = true;
					continue;
				}
				else parsingCommentLineIndex++;	//	parsing forward
			}
			else parsingCommentLineIndex = 0;	//	parsing line entry fail so reset parameter

			output.push_back(currentChar);
		}
		else if (!commentLineEntry.empty() && currentChar == commentLineEntry[parsingCommentLineIndex])
		{
			parsingCommentBlockIndex = 0;	//	parsing block fail so reset parameter
			if (parsingCommentLineIndex >= commentLineEntry.size() - 1)	//	comment line entry string match entirely -> begin parsing a comment line
			{
				//	erase last pushed char coresponding to the comment line entry string
				output.erase(std::prev(output.end(), parsingCommentLineIndex), output.end());

				//	set parameters for parsing a line
				parsingCommentBlockIndex = 0;
				parsingCommentLineIndex = 0;
				actuallyParsingCommentBlock = false;
				actuallyParsingCommentLine = true;
				continue;
			}
			else parsingCommentLineIndex++;	//	parsing forward

			output.push_back(currentChar);	//	not begining parsing comment block or line so push current char to output string
		}
		else
		{
			//	all parsing test fail so reset parameters
			parsingCommentBlockIndex = 0;
			parsingCommentLineIndex = 0;
			actuallyParsingCommentBlock = false;
			actuallyParsingCommentLine = false;

			//	not begining parsing comment block or line so push current char to output string
			output.push_back(currentChar);
		}
	}

	//	end
	file.close();
	return output;
}
void ToolBox::clearWhitespace(std::string& input)
{
	for (std::string::iterator it = input.begin(); it != input.end();)
	{
		if (std::isspace(*it) && *std::prev(it) == ' ' || std::isspace(*it) && it == input.begin())
			it = input.erase(it);
		else if (std::isspace(*it) && *std::prev(it) != ' ')
		{
			*it = ' ';
			++it;
		}
		else ++it;
	}
}
bool ToolBox::isPathExist(const std::string& fileName)
{
	struct stat buffer;
	return (stat(fileName.c_str(), &buffer) == 0);
}


Variant ToolBox::getFromVec2f(const vec2f& vec)
{
	Variant v = Variant(Variant::ArrayType());
	for (int i = 0; i < 2; i++)
		v.getArray().push_back(Variant(vec[i]));
	return v;
}
Variant ToolBox::getFromVec3f(const vec3f& vec)
{
	Variant v = Variant(Variant::ArrayType());
	for (int i = 0; i < 3; i++)
		v.getArray().push_back(Variant(vec[i]));
	return v;
}
Variant ToolBox::getFromVec4f(const vec4f& vec)
{
	Variant v = Variant(Variant::ArrayType());
	for (int i = 0; i < 4; i++)
		v.getArray().push_back(Variant(vec[i]));
	return v;
}
Variant ToolBox::getFromQuatf(const quatf& quat)
{
	Variant v = Variant(Variant::ArrayType());
	for (int i = 0; i < 4; i++)
		v.getArray().push_back(Variant(quat[i]));
	return v;
}
Variant ToolBox::getFromMat4f(const mat4f& mat)
{
	Variant v = Variant(Variant::ArrayType());
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			v.getArray().push_back(Variant(mat[i][j]));
	return v;
}


void ToolBox::optimizeStaticMesh(std::vector<vec4f>& verticesArray,
	std::vector<vec4f>& normalesArray,
	std::vector<vec4f>& colorArray,
	std::vector<unsigned short>&facesArray)
{
	std::vector<vec4f> verticesBuffer;
	std::vector<vec4f> normalesBuffer;
	std::vector<vec4f> colorBuffer;
	std::vector<unsigned short> facesBuffer;

	//	an ordered vertex data (position, normal and color)
	struct OrderedVertex
	{
		vec4f position;
		vec4f normal;
		vec4f color;

		int inf(const vec4f& l, const vec4f& r) const
		{
			//	return 1 if equals, 2 if l < r and 3 if l > r
			if (l.x != r.x) return (l.x < r.x ? 2 : 3);
			if (l.y != r.y) return (l.y < r.y ? 2 : 3);
			if (l.z != r.z) return (l.z < r.z ? 2 : 3);
			return 1;
		}
		bool operator< (const OrderedVertex& r) const
		{
			int res = inf(position, r.position);
			if (res == 2) return true;
			if (res == 3) return false;

			res = inf(normal, r.normal);
			if (res == 2) return true;
			if (res == 3) return false;

			res = inf(color, r.color);
			if (res == 2) return true;
			return false;
		};
	};

	std::map<OrderedVertex, unsigned short> vertexAlias;
	std::map<OrderedVertex, unsigned short>::iterator alias;
	OrderedVertex current;

	for (unsigned int i = 0; i < facesArray.size(); i++)
	{
		current.position = verticesArray[facesArray[i]];
		current.normal = normalesArray[facesArray[i]];
		current.color = colorArray[facesArray[i]];

		alias = vertexAlias.find(current);
		if (alias == vertexAlias.end())
		{
			verticesBuffer.push_back(verticesArray[facesArray[i]]);
			normalesBuffer.push_back(normalesArray[facesArray[i]]);
			colorBuffer.push_back(colorArray[facesArray[i]]);
			facesBuffer.push_back((unsigned short)vertexAlias.size());

			vertexAlias[current] = (unsigned short)vertexAlias.size();
		}
		else facesBuffer.push_back(alias->second);
	}

	verticesArray.swap(verticesBuffer);
	normalesArray.swap(normalesBuffer);
	colorArray.swap(colorBuffer);
	facesArray.swap(facesBuffer);
}
void ToolBox::optimizeHullMesh(Mesh* mesh)
{
	//	copy current array of mesh
	const std::vector<vec4f>& verticesArray = *mesh->getVertices();
	const std::vector<vec4f>& normalesArray = *mesh->getNormals();
	const std::vector<unsigned short>& facesArray = *mesh->getFaces();

	// create result output
	std::vector<vec4f> verticesBuffer;
	std::vector<vec4f> normalBuffer;
	std::vector<unsigned short> facesBuffer;

	//	struct to be able to sort vertices
	struct OrderedVertex
	{
		vec4f position;
		int inf(const vec4f& l, const vec4f& r) const
		{
			//	return 1 if equals, 2 if l < r and 3 if l > r
			if (l.x != r.x) return (l.x < r.x ? 2 : 3);
			if (l.y != r.y) return (l.y < r.y ? 2 : 3);
			if (l.z != r.z) return (l.z < r.z ? 2 : 3);
			return 1;
		}
		bool operator< (const OrderedVertex& r) const
		{
			int res = inf(position, r.position);
			if (res == 2) return true;
			if (res == 3) return false;
			return false;
		};
	};

	//	association table "vertex <-> index"
	std::map<OrderedVertex, unsigned short> aliases;
	std::map<OrderedVertex, unsigned short>::iterator alias;
	OrderedVertex current;

	for (unsigned int i = 0; i < facesArray.size(); i++)
	{
		current.position = verticesArray[facesArray[i]];
		if ((i % 3) == 0)
			normalBuffer.push_back(normalesArray[facesArray[i]]);

		alias = aliases.find(current);
		if (alias == aliases.end())	// no alias found for current vertex (so create one)
		{
			aliases[current] = (unsigned short)verticesBuffer.size();
			facesBuffer.push_back((unsigned short)verticesBuffer.size());
			verticesBuffer.push_back(verticesArray[facesArray[i]]);
		}
		else facesBuffer.push_back(alias->second); // use the alias found instead
	}

	//	replace mesh arrays
	mesh->uvs.clear();
	mesh->vertices = verticesBuffer;
	mesh->normals = normalBuffer;
	mesh->faces = facesBuffer;
}
//