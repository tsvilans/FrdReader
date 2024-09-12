#include "frd_reader.h"

int frd_reader2::get_line(char* ptr, char* end)
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

void frd_reader2::parse_result_block(std::ifstream stream)
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

void frd_reader2::read(const char* frd_path)
{
	std::ifstream stream;
	stream.open(frd_path, std::ios::binary);

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
	mEnd = ptr + length * sizeof(char);
	bool kill = false;


	/*
	The results buffer can just be a 2D array.
	
	                                    |STEP 1
	                                    |DISP                   |TOSTRAIN
	node id     |X      |Y      |Z      |DX     |DY     |DZ     |EXX    |EYY
	------------|-------|-------|-------|-------|-------|-------|-------|-------|
	11
	12
	13
	14
	
	To find address of a node, it is the buffer start plus 
	the total step + field + component number offset (next row).
	
	*/

	// Scan number of datasets and number of values

	int line_length = 0;
	int channels = 4; // node_id, X, Y, and Z
	std::vector<size_t> dataset_positions;

	while (ptr < mEnd && !kill)
	{
		line_length = get_line(ptr, mEnd);
		if (line_length < 6)
		{
			ptr += line_length;
			continue;
		}

		// Get block code
		std::string code(ptr, 6);
		ltrim(code);
		rtrim(code);

		// End of file!
		if (std::strcmp(code.c_str(), "9999") == 0) break;


	}

	// Reset pointer to beginning
	ptr = buffer;
	while (ptr < mEnd && !kill)
	{
		// read buffer until next linebreak
		line_length = get_line(ptr, mEnd);

		// If not enough characters to read a code...
		if (line_length < 6)
		{
			ptr += line_length;
			continue;
		}
		std::string code(ptr, 6);
		ltrim(code);
		rtrim(code);

		if (std::strcmp(code.c_str(), "9999") == 0)
		{
			break;
		}
		else if (std::strcmp(code.c_str(), "1C") == 0)
		{
			std::cout << "Reading model header..." << std::endl;
			ptr += line_length;
			read_header(ptr);
		}
		else if (std::strcmp(code.c_str(), "2C") == 0) // nodes
		{
			num_nodes = std::stoi(std::string(ptr + 6, 30));
			std::cout << num_nodes << " nodes found." << std::endl;

			int format = std::stoi(std::string(ptr + 36, 38));

			ptr += line_length;
			switch (format)
			{
			case(1):
				read_nodes_ascii(ptr);
				break;
			default:
				read_nodes_binary(ptr);
				break;
			}
		}
		else if (std::strcmp(code.c_str(), "3C") == 0) // element block
		{
			num_elements = std::stoi(std::string(ptr + 6, 30));
			std::cout << num_elements << " elements found." << std::endl;

			int format = std::stoi(std::string(ptr + 36, 38));

			ptr += line_length;
			switch (format)
			{
			case(1):
				read_elements_ascii(ptr);
				break;
			default:
				read_elements_binary(ptr);
				break;
			}
		}
		else if (std::strcmp(code.c_str(), "1P") == 0) // step block
		{
			int resultId = std::stoi(std::string(ptr + 10, 26));


			ptr += line_length;
			int line_length = get_line(ptr, mEnd);

			// Now the code should be 100C
			code.assign(ptr, 6);
			ltrim(code);

			if (std::strcmp(code.c_str(), "100C") == 0) // results block
			{
				// Extract results header
				std::string setName(ptr + 6, 6);
				float value = std::stof(std::string(ptr + 12, 12));
				int nValues = std::stoi(std::string(ptr + 24, 12));

				if (num_nodes != nValues)
				{
					std::cout << "Number of values doesn't correspond to number of nodes...\nThis is very confusing. Breaking..." << std::endl;
					break;
				}

				std::string text(ptr + 36, 20);

				//std::cout << "Result text: " << text << std::endl;

				int icType = std::stoi(std::string(ptr + 56, 2));
				int nStep = std::stoi(std::string(ptr + 58, 5));
				std::string analysis(ptr + 63, 10);

				int format = std::stoi(std::string(ptr + 73, 2));


				std::cout << "icType: " << icType << ", nStep: " << nStep << ", analysis: " << analysis << ", format: " << format << std::endl;

				// Next line, e.g.  -4  TOSTRAIN    6    1
				ptr += line_length;
				line_length = get_line(ptr, mEnd);

				std::string name(ptr + 5, 8);
				ltrim(name);
				rtrim(name);


				int numComponents = std::stoi(std::string(ptr + 13, 5));
				int irType = std::stoi(std::string(ptr + 18, 5));

				frd_results_block header;
				header.name = name;
				header.format = format;
				header.numComponents = numComponents;
				header.ictype = icType;
				header.nstep = nStep;

				mMetadata[nStep][name] = header;
				std::cout << name << ", num components: " << numComponents << ", type: " << irType << std::endl;

				ptr += line_length;

				std::vector<std::string> componentNames;

				for (int i = 0; i < numComponents; ++i)
				{
					line_length = get_line(ptr, mEnd);

					std::string componentName(ptr + 5, 8);
					ltrim(componentName);
					rtrim(componentName);

					componentNames.push_back(componentName);

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
							std::string extraName(ptr + 33, 8);
							ltrim(extraName);
						}
					}
					ptr += line_length;
				}

				std::vector<std::map<int, float>> values;

				for (int i = 0; i < numComponents; ++i)
				{
					values.push_back(std::map<int, float>());
				}

				switch (format)
				{
				case(1):
					for (int i = 0; i < num_nodes; ++i)
					{
						int nodeId = std::stoi(std::string(ptr + 3, 10));

						for (int j = 0; j < numComponents; ++j)
						{
							values[j][nodeId] = std::stof(std::string(ptr + 13 + 12 * j, 12));
						}

						line_length = get_line(ptr, mEnd);
						ptr += line_length;
					}

					break;
				default:
					for (int i = 0; i < num_nodes; ++i)
					{
						int nodeId = *(int*)ptr;

						ptr += sizeof(int);
						for (int j = 0; j < numComponents; ++j)
						{
							float v = *(float*)ptr;
							values[j][nodeId] = v;

							ptr += sizeof(float);
						}
					}

					break;
				}

				for (int i = 0; i < numComponents; ++i)
				{
					mValues[nStep][name][componentNames[i]] = values[i];
				}
			}
		}
		else
		{
			std::cout << "Unrecognized code encountered: '" << code << "'" << std::endl;
			break;
		}
	}

	delete[] buffer;

	std::cout << "Done." << std::endl;
}

int frd_reader2::get(size_t step_id, std::string field, std::string component, size_t node_id, double &result)
{
	size_t step_offset, field_offset, component_offset;
	for (size_t i = 0; i < step_ids.size(); ++i)
	{
		if (step_ids[i] == step_id)
		{
			step_offset = step_offsets[i];
		}
		break;
	}

	for (size_t i = 0; i < field_names.size(); ++i)
	{
		if (strcmp(field_names[i].c_str(), field.c_str()) == 0)
		{
			field_offset = field_offsets[i];
		}
		break;
	}

	for (size_t i = 0; i < component_names.size(); ++i)
	{
		if (strcmp(component_names[i].c_str(), component.c_str()) == 0)
		{
			component_offset = component_offsets[i];
		}
		break;
	}

	if (step_offset < 1 || field_offset < 1 || component_offset < 1)
		return -1;

	size_t i = 0;
	while (i < num_nodes)
	{
		if (static_cast<size_t>(data[i * row_step]) == node_id)
		{
			result = data[i * row_step + step_offset + field_offset + component_offset];
			return 0;
		}
		i++;
	}

	return -1;
}

int frd_reader2::get(size_t step_id, std::string field, std::string component, std::vector<size_t> node_ids, std::vector<double> results)
{

	size_t step_offset, field_offset, component_offset;
	for (size_t i = 0; i < step_ids.size(); ++i)
	{
		if (step_ids[i] == step_id)
		{
			step_offset = step_offsets[i];
		}
		break;
	}

	for (size_t i = 0; i < field_names.size(); ++i)
	{
		if (strcmp(field_names[i].c_str(), field.c_str()) == 0)
		{
			field_offset = field_offsets[i];
		}
		break;
	}

	for (size_t i = 0; i < component_names.size(); ++i)
	{
		if (strcmp(component_names[i].c_str(), component.c_str()) == 0)
		{
			component_offset = component_offsets[i];
		}
		break;
	}

	if (step_offset < 1 || field_offset < 1 || component_offset < 1)
		return -1;

	size_t i = 0;
	for (int j = 0; j < node_ids.size(); ++j)
	{
		while (i < num_nodes)
		{
			if (static_cast<size_t>(data[i * row_step]) == node_ids[j])
			{
				results.push_back(data[i * row_step + step_offset + field_offset + component_offset]);
				return 0;
			}
			i++;
		}
	}

	return -1;
}

void frd_reader2::read_header(char*& ptr)
{
	std::string code;
	code.assign(ptr, 6);
	ltrim(code);

	while (std::strcmp(code.c_str(), "1U") == 0)
	{
		int line_length = get_line(ptr, mEnd);
		std::string header_data(ptr + 6, 66);

		ptr += line_length;

		code.assign(ptr, 6);
		ltrim(code);

		std::cout << header_data << std::endl;
	}
}

void frd_reader2::read_nodes_ascii(char*& ptr)
{
	for (size_t i = 0; i < num_nodes; ++i)
	{
		int line_length = get_line(ptr, mEnd);
		int id = std::stoi(std::string(ptr + 3, 10));
		double x = std::stod(std::string(ptr + 13, 12));
		double y = std::stod(std::string(ptr + 25, 12));
		double z = std::stod(std::string(ptr + 37, 12));

		mNodes[id] = frd_node(id, x, y, z);
		ptr += line_length;
	}
}
void frd_reader2::read_nodes_binary(char*& ptr)
{
	frd_node* nodes_ptr = (frd_node*)ptr;
	for (size_t i = 0; i < num_nodes; ++i)
	{
		mNodes[nodes_ptr[i].id] = nodes_ptr[i];
	}

	ptr += num_nodes * sizeof(frd_node);
	std::cout << "Finished reading nodes." << std::endl;
}

void frd_reader2::read_elements_ascii(char*& ptr)
{
	int nIndices;
	for (size_t i = 0; i < num_elements; ++i)
	{
		int line_length = get_line(ptr, mEnd);
		int lineId = std::stoi(std::string(ptr, 3));

		frd_element_header header;
		header.id = std::stoi(std::string(ptr + 3, 10));
		header.type = std::stoi(std::string(ptr + 13, 5));
		header.group = std::stoi(std::string(ptr + 18, 5));
		header.material = std::stoi(std::string(ptr + 23, 5));

		nIndices = ELEMENT_TYPE_MAP[header.type];

		frd_element element;
		element.header = header;
		element.indices.resize(nIndices);


		ptr += line_length;
		line_length = get_line(ptr, mEnd);

		// Parse element indices
		for (int i = 0; i < nIndices; ++i)
		{
			element.indices[i] = std::stoi(std::string(ptr + 3 + 10 * i, 10));
		}

		mElements[header.id] = element;

		ptr += line_length;
	}
}
void frd_reader2::read_elements_binary(char*& ptr)
{
	int nIndices;
	for (int i = 0; i < num_elements; ++i)
	{
		frd_element_header header = *(frd_element_header*)ptr;
		ptr += sizeof(frd_element_header);

		nIndices = ELEMENT_TYPE_MAP[header.type];

		frd_element element;
		element.header = header;
		element.indices = std::vector<int>((int*)ptr, (int*)ptr + nIndices);

		mElements[header.id] = element;

		ptr += nIndices * sizeof(int);
	}

	std::cout << "Finished reading elements." << std::endl;
}

void frd_reader2::read_results_ascii(char*& ptr, int nComponents)
{
	for (int i = 0; i < num_nodes; ++i)
	{
		int line_length = get_line(ptr, mEnd);
		std::string line(ptr, line_length);
		rtrim(line);

		//std::cout << line << std::endl;

		ptr += line_length;
	}

}

void frd_reader2::read_results_binary(char*& ptr, int nComponents)
{
	//Dictionary<System::String^, array<float>^>^ ComponentData = gcnew Dictionary<System::String^, array<float>^>();
	//array<array<float>^>^ values = gcnew array<array<float>^>(numComponents);

	for (int i = 0; i < nComponents; ++i)
	{
		//values[i] = gcnew array<float>(numAllNodes);
	}

	for (int i = 0; i < num_nodes; ++i)
	{
		int nodeId = *(int*)ptr;
		//int nodeIndex = nodeMap[nodeId];

		ptr += sizeof(int);
		for (int j = 0; j < nComponents; ++j)
		{
			float v = *(float*)ptr;
			//values[j][nodeIndex] = v;

			ptr += sizeof(float);
		}
	}
}