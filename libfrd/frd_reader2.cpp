// frd_reader2.cpp

#include "frd_reader.h"
#include <iostream>   // For std::cout, std::cerr
#include <fstream>    // For std::ifstream
#include <map>        // For std::map
#include <vector>     // For std::vector
#include <string>     // For std::string, std::stoi, std::stof
#include "util.h"     // Ensure this is included for ltrim and rtrim

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

void frd_reader2::parse_result_block(std::ifstream &stream)
{
    // Using std::vector for buffer management
    std::vector<char> buffer(6);
    stream.read(buffer.data(), 6);
    std::string name(buffer.begin(), buffer.end());

    buffer.resize(12);
    stream.read(buffer.data(), 12);
    std::string value(buffer.begin(), buffer.end());

    buffer.resize(12);
    stream.read(buffer.data(), 12);
    int nNodes = std::stoi(std::string(buffer.begin(), buffer.end()));
}

void frd_reader2::read(const char* frd_path)
{
    std::ifstream stream(frd_path, std::ios::binary);
    if (!stream) {
        std::cerr << "Error: Unable to open file " << frd_path << std::endl;
        return;
    }

    // Get length of file
    stream.seekg(0, std::ios::end);
    long length = stream.tellg();
    stream.seekg(0, std::ios::beg);

    // Use std::vector for buffer management
    std::vector<char> buffer(length);
    stream.read(buffer.data(), length);
    stream.close();

    // Setup buffer parsing
    char* ptr = buffer.data();
    mEnd = ptr + length;
    bool kill = false;

    // Initial scan (if needed)
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
        if (code == "9999") break;

        // Implement additional initial scan logic if necessary
        // ...
    }

    // Reset pointer to beginning
    ptr = buffer.data();
    while (ptr < mEnd && !kill)
    {
        // Read buffer until next linebreak
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

        if (code == "9999")
        {
            break;
        }
        else if (code == "1C")
        {
            std::cout << "Reading model header..." << std::endl;
            ptr += line_length;
            read_header(ptr);
        }
        else if (code == "2C") // nodes
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
        else if (code == "3C") // element block
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
        else if (code == "1P") // step block
        {
            int resultId = std::stoi(std::string(ptr + 10, 26));

            ptr += line_length;
            line_length = get_line(ptr, mEnd);

            // Now the code should be 100C
            code.assign(ptr, 6);
            ltrim(code);

            if (code == "100C") // results block
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

                // Assign to mMetadata with int key
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
                    values.emplace_back();
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
                            int nodeId = *reinterpret_cast<int*>(ptr);
                            ptr += sizeof(int);
                            for (int j = 0; j < numComponents; ++j)
                            {
                                float v = *reinterpret_cast<float*>(ptr);
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
        std::cout << "Done." << std::endl;
    }
}

int frd_reader2::get(size_t step_id, const std::string& field, const std::string& component, size_t node_id, double &result)
{
    size_t step_offset = 0, field_offset = 0, component_offset = 0;
    bool found = false;

    for (size_t i = 0; i < step_ids.size(); ++i)
    {
        if (step_ids[i] == step_id)
        {
            step_offset = step_offsets[i];
            found = true;
            break; // Correctly placed break inside the if
        }
    }

    if (!found) return -1;

    found = false;
    for (size_t i = 0; i < field_names.size(); ++i)
    {
        if (field_names[i] == field)
        {
            field_offset = field_offsets[i];
            found = true;
            break; // Correctly placed break inside the if
        }
    }

    if (!found) return -1;

    found = false;
    for (size_t i = 0; i < component_names.size(); ++i)
    {
        if (component_names[i] == component)
        {
            component_offset = component_offsets[i];
            found = true;
            break; // Correctly placed break inside the if
        }
    }

    if (!found)
        return -1;

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

int frd_reader2::get(size_t step_id, const std::string& field, const std::string& component, const std::vector<size_t>& node_ids, std::vector<double> &results)
{
    size_t step_offset = 0, field_offset = 0, component_offset = 0;
    bool found = false;

    for (size_t i = 0; i < step_ids.size(); ++i)
    {
        if (step_ids[i] == step_id)
        {
            step_offset = step_offsets[i];
            found = true;
            break; // Correctly placed break inside the if
        }
    }

    if (!found)
        return -1;

    found = false;
    for (size_t i = 0; i < field_names.size(); ++i)
    {
        if (field_names[i] == field)
        {
            field_offset = field_offsets[i];
            found = true;
            break; // Correctly placed break inside the if
        }
    }

    if (!found)
        return -1;

    found = false;
    for (size_t i = 0; i < component_names.size(); ++i)
    {
        if (component_names[i] == component)
        {
            component_offset = component_offsets[i];
            found = true;
            break; // Correctly placed break inside the if
        }
    }

    if (!found)
        return -1;

    if (step_offset < 1 || field_offset < 1 || component_offset < 1)
        return -1;

    size_t i = 0;
    for (size_t j = 0; j < node_ids.size(); ++j)
    {
        while (i < num_nodes)
        {
            if (static_cast<size_t>(data[i * row_step]) == node_ids[j])
            {
                results.push_back(data[i * row_step + step_offset + field_offset + component_offset]);
                break; // Found the node, move to next node_id
            }
            i++;
        }
    }

    return 0;
}

void frd_reader2::read_header(char*& ptr)
{
    std::string code;
    code.assign(ptr, 6);
    ltrim(code);
    rtrim(code);

    while (code == "1U")
    {
        int line_length = get_line(ptr, mEnd);
        std::string header_data(ptr + 6, 66);

        ptr += line_length;

        code.assign(ptr, 6);
        ltrim(code);
        rtrim(code);

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
    frd_node* nodes_ptr = reinterpret_cast<frd_node*>(ptr);
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
        for (int j = 0; j < nIndices; ++j)
        {
            element.indices[j] = std::stoi(std::string(ptr + 3 + 10 * j, 10));
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
        frd_element_header header = *reinterpret_cast<frd_element_header*>(ptr);
        ptr += sizeof(frd_element_header);

        nIndices = ELEMENT_TYPE_MAP[header.type];

        frd_element element;
        element.header = header;
        element.indices = std::vector<int>(reinterpret_cast<int*>(ptr), reinterpret_cast<int*>(ptr) + nIndices);

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

        // Optionally process the line
        // std::cout << line << std::endl;

        ptr += line_length;
    }
}

void frd_reader2::read_results_binary(char*& ptr, int nComponents)
{
    // Placeholder for binary results processing
    for (int i = 0; i < nComponents; ++i)
    {
        // Implement binary results reading if needed
    }

    for (int i = 0; i < num_nodes; ++i)
    {
        int nodeId = *reinterpret_cast<int*>(ptr);
        ptr += sizeof(int);
        for (int j = 0; j < nComponents; ++j)
        {
            float v = *reinterpret_cast<float*>(ptr);
            // Process 'v' as needed
            ptr += sizeof(float);
        }
    }
}
