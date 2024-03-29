#pragma once
#include <msclr\marshal_cppstd.h>
#include <fstream>
#include <string>
#include <map>

using namespace System;
using System::Runtime::InteropServices::Marshal;
using System::Collections::Generic::List;
using System::Collections::Generic::Dictionary;

#pragma pack (1)
struct frd_node
{
	int id;
	double x, y, z;
};
#pragma pack (0)

#pragma pack (1)
struct frd_element_header
{
	int id, type, group, material;
};
#pragma pack (0)


inline std::string ltrim(std::string& str)
{
	auto it2 = std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace(static_cast<unsigned char>(ch)); });
	str.erase(str.begin(), it2);
	return str;
}

// trim from end (in place)
inline std::string rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
	return s;
}

inline std::string trim(std::string s)
{
	rtrim(s);
	ltrim(s);
	return s;
}

void parse_result_block(std::ifstream stream)
{
	char* buffer = new char[6];
	stream.read(buffer, 6);
	std::string name(buffer);

	buffer = new char[12];
	stream.read(buffer, 12);
	std::string value(buffer);

	buffer = new char[12];
	stream.read(buffer, 12);
	int nNodes = std::stoi(std::string(buffer));
}

int get_line(char* ptr, char* end)
{
	int line_length = 0;

	while (ptr < end)
	{
		line_length++;
		if (*ptr == '\n') break;
		ptr++;
	}
	return line_length;
}


namespace FrdReader {

	public ref class FrdNode
	{

	public:
		FrdNode(int id, double x, double y, double z)
		{
			Id = id;
			X = x;
			Y = y;
			Z = z;
		}

		double X, Y, Z;
		int Id;
	};

	public ref class FrdElement
	{

	public:
		FrdElement(int id, int type, array<int>^ indices)
		{
			Id = id;
			Type = type;
			Indices = indices;
		}

		array<int>^ Indices;
		int Id, Type;
	};

	public ref class FrdResults
	{
	public:
		FrdResults()
		{
			Nodes = gcnew List<FrdNode^>();
			Elements = gcnew List<FrdElement^>();
			HeaderData = gcnew List<System::String^>();
			Fields = gcnew Dictionary < System::String^, Dictionary<System::String^, array<float>^>^>();
			ElementTypeMap = gcnew Dictionary<int, int>();

			ElementTypeMap[1] = 8;
			ElementTypeMap[2] = 6;
			ElementTypeMap[3] = 4;
			ElementTypeMap[4] = 20;
			ElementTypeMap[5] = 15;
			ElementTypeMap[6] = 10;
			ElementTypeMap[7] = 3;
			ElementTypeMap[8] = 6;
			ElementTypeMap[9] = 4;
			ElementTypeMap[10] = 8;
			ElementTypeMap[11] = 2;
			ElementTypeMap[12] = 3;

		}

		void Read(System::String^ frd_path)
		{
			std::string frd_path_c = msclr::interop::marshal_as<std::string>(frd_path);
			std::ifstream stream;

			stream.open(frd_path_c.c_str(), std::ios::binary);

			// Get length of file
			stream.seekg(0, std::ios::end);
			long length = stream.tellg();
			stream.seekg(0, std::ios::beg);

			char* buffer = new char[length];

			// Read results file
			stream.read(buffer, length);

			// Close file
			stream.close();

			// Setup buffer parsing
			char* ptr = buffer;
			char* end = ptr + length * sizeof(char);

			bool kill = false;

			std::map<int, int> nodeMap;

			// Scan buffer
			while (ptr < end && !kill)
			{

				// read buffer until next linebreak
				int line_length = get_line(ptr, end);

				// If not enough characters to read a code...
				if (line_length < 6)
				{
					ptr += line_length;
					continue;
				}

				std::string code = ltrim(std::string(ptr, 6));

				if (std::strcmp(code.c_str(), "1C") == 0)
				{
					ptr += line_length;
					continue;
				}
				else if (std::strcmp(code.c_str(), "1U") == 0)
				{
					HeaderData->Add(gcnew System::String(ptr, 6, line_length - 6));
					ptr += line_length;
				}
				else if (std::strcmp(code.c_str(), "3C") == 0)
				{
					long nElements = std::stol(ltrim(std::string(ptr + 6, 30)));
					int blockFormat = std::stol(ltrim(std::string(ptr + 36, line_length - 36)));

					ptr += line_length;
					int nIndices = 8;
					for (int i = 0; i < nElements; ++i)
					{
						frd_element_header hdr = *(frd_element_header*)ptr;
						ptr += sizeof(frd_element_header);

						nIndices = ElementTypeMap[hdr.type];

						array<int>^ indices = gcnew array<int>(nIndices);
						Marshal::Copy(IntPtr(ptr), indices, 0, nIndices);

						Elements->Add(gcnew FrdElement(hdr.id, hdr.type, indices));
						ptr += nIndices * sizeof(int);
					}
				}
				else if (std::strcmp(code.c_str(), "2C") == 0)
				{
					long nNodes = std::stol(ltrim(std::string(ptr + 6, 30)));

					//throw gcnew Exception(System::String::Format("number of nodes: {0}", nNodes));

					ptr += line_length;
					frd_node* nodes_c = (frd_node*)ptr;
					Nodes = gcnew System::Collections::Generic::List<FrdNode^>(nNodes);

					for (int i = 0; i < nNodes; ++i)
					{
						nodeMap[nodes_c[i].id] = i;
						Nodes->Add(gcnew FrdNode(nodes_c[i].id, nodes_c[i].x, nodes_c[i].y, nodes_c[i].z));
					}

					ptr += nNodes * sizeof(frd_node);
				}
				else if (std::strcmp(code.c_str(), "1P") == 0)
				{
					ptr += line_length;
					continue;
				}
				else if (std::strcmp(code.c_str(), "100C") == 0)
				{
					int nNodes = std::stoi(std::string(ptr + 24, 12));

					ptr += line_length;
					line_length = get_line(ptr, end);
					std::string name = rtrim(ltrim(std::string(ptr + 5, 8)));

					List<System::String^>^ componentsList = gcnew List<System::String^>();

					int numComponents = std::stoi(std::string(ptr + 13, 5));
					int irType = std::stoi(std::string(ptr + 18, 5));

					ptr += line_length;

					for (int i = 0; i < numComponents; ++i)
					{
						line_length = get_line(ptr, end);

						std::string componentName = rtrim(ltrim(std::string(ptr + 5, 8)));

						componentsList->Add(gcnew System::String(componentName.c_str(), 0, componentName.size()));

						int icMenu = std::stoi(std::string(ptr + 13, 5));
						int icType = std::stoi(std::string(ptr + 18, 5));
						int icIndex1 = std::stoi(std::string(ptr + 23, 5));
						int icIndex2 = std::stoi(std::string(ptr + 28, 5));
						if (line_length > 34)
						{
							int iExist = std::stoi(std::string(ptr + 33, 5));

							if (iExist > 0) numComponents--;
							if (line_length >= 41)
							{
								std::string extraName = ltrim(std::string(ptr + 33, 8));
							}
						}
						ptr += line_length;
					}

					Dictionary<System::String^, array<float>^>^ ComponentData = gcnew Dictionary<System::String^, array<float>^>();
					array<array<float>^>^ values = gcnew array<array<float>^> (numComponents);
					
					for (int i = 0; i < numComponents; ++i)
					{
						values[i] = gcnew array<float>(nNodes);
					}

					for (int i = 0; i < nNodes; ++i)
					{
						int nodeId = *(int*)ptr;
						int nodeIndex = nodeMap[nodeId];

						ptr += sizeof(int);
						for (int j = 0; j < numComponents; ++j)
						{
							float v = *(float*)ptr;
							values[j][nodeIndex] = v;

							ptr += sizeof(float);
						}
					}

					for (int i = 0; i < numComponents; ++i)
					{
						ComponentData[componentsList[i]] = values[i];
					}

					Fields->Add(gcnew System::String(name.c_str(), 0, name.size()), ComponentData);


				}
				else
				{
					ptr += line_length;
				}
			}

			delete[] buffer;
		}

		List<FrdNode^>^ Nodes;
		List<FrdElement^>^ Elements;
		List<System::String^>^ HeaderData;
		Dictionary<System::String^, Dictionary<System::String^, array<float>^>^>^ Fields;
		Dictionary<int, int>^ ElementTypeMap;

	};
}
