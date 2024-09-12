#pragma once
#include <msclr\marshal_cppstd.h>
#include <fstream>
#include <string>
#include <map>

#include "frd_reader.h"

using namespace System;
using System::Runtime::InteropServices::Marshal;
using System::Collections::Generic::List;
using System::Collections::Generic::Dictionary;

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
		FrdElement(int id, int type, int group, int material, array<int>^ indices)
		{
			Id = id;
			Type = type;
			Group = group;
			Material = material;
			Indices = indices;
		}

		array<int>^ Indices;
		int Id, Type, Group, Material;
	};

	public ref class FrdResults
	{
	public:
		FrdResults()
		{
			Nodes = gcnew List<FrdNode^>();
			Elements = gcnew List<FrdElement^>();
			HeaderData = gcnew List<System::String^>();
			Fields = gcnew Dictionary < int, Dictionary < System::String^, Dictionary<System::String^, array<float>^>^>^>();
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
			frd_reader reader;

			reader.read(frd_path_c.c_str());

			size_t nNodes = reader.mNodes.size();

			// Add nodes
			Nodes->Clear();
			for (auto iter = reader.mNodes.begin(); iter != reader.mNodes.end(); iter++)
			{
				int node_id = (int)iter->first;
				frd_node node = iter->second;
				Nodes->Add(gcnew FrdNode(node_id, node.x, node.y, node.z));
				//std::cout << "n : " << node_id << " (" << node.x << ", " << node.y << ", " << node.z << ")" << std::endl;
			}

			// Add elements
			Elements->Clear();
			for (auto iter = reader.mElements.begin(); iter != reader.mElements.end(); iter++)
			{
				int element_id = (int)iter->first;
				frd_element element = iter->second;
				array<int>^ indices = gcnew array<int>(element.indices.size());
				for (int i = 0; i < indices->Length; ++i)
				{
					indices[i] = element.indices[i];
				}
				//std::cout << "e : " << element_id << " (" << indices[0] << ", " << indices[1] << ", " << indices[2] << ", ... )" << std::endl;

				Elements->Add(gcnew FrdElement(element_id, element.header.type, element.header.group, element.header.material, indices));
			}

			// Add results
			Fields->Clear();
			for (auto step = reader.mValues.begin(); step != reader.mValues.end(); step++)
			{
				auto stepCommon = gcnew Dictionary < System::String^, Dictionary < System::String^, array<float>^>^>();
				for (auto field = step->second.begin(); field != step->second.end(); field++)
				{
					auto fieldCommon = gcnew Dictionary < System::String^, array<float>^>();

					for (auto component = field->second.begin(); component != field->second.end(); component++)
					{
						size_t i = 0;
						array<float>^ componentValues = gcnew array<float>(nNodes);
						for (auto value = component->second.begin(); value != component->second.end(); value++)
						{
							componentValues[i] = value->second;
							i++;
						}
						fieldCommon->Add(gcnew System::String(component->first.c_str(), 0, component->first.size()), componentValues);
					}

					stepCommon->Add(gcnew System::String(field->first.c_str(), 0, field->first.size()), fieldCommon);
				}
				Fields->Add(step->first, stepCommon);
			}
		}

		/*
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

			long numAllNodes = -1;

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

						Elements->Add(gcnew FrdElement(hdr.id, hdr.type, hdr.group, hdr.material, indices));
						ptr += nIndices * sizeof(int);
					}
				}
				else if (std::strcmp(code.c_str(), "2C") == 0)
				{
					numAllNodes = std::stol(ltrim(std::string(ptr + 6, 30)));

					//throw gcnew Exception(System::String::Format("number of nodes: {0}", nNodes));

					ptr += line_length;
					frd_node* nodes_c = (frd_node*)ptr;
					Nodes = gcnew System::Collections::Generic::List<FrdNode^>(numAllNodes);

					for (int i = 0; i < numAllNodes; ++i)
					{
						nodeMap[nodes_c[i].id] = i;
						Nodes->Add(gcnew FrdNode(nodes_c[i].id, nodes_c[i].x, nodes_c[i].y, nodes_c[i].z));
					}

					ptr += numAllNodes * sizeof(frd_node);
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
						values[i] = gcnew array<float>(numAllNodes);
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
		*/

		List<FrdNode^>^ Nodes;
		List<FrdElement^>^ Elements;
		List<System::String^>^ HeaderData;
		Dictionary < int, Dictionary < System::String^, Dictionary<System::String^, array<float>^>^>^>^ Fields;
		Dictionary<int, int>^ ElementTypeMap;

	};
}
