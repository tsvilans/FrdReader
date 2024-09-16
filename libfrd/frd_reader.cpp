#include "frd_reader.h"

int frd_reader::get_line(char* ptr, char* end) {
    int line_length = 0;
    while (ptr < end) {
        line_length++;
        if (*ptr == '\n') break;
        ptr++;
    }
    return line_length;
}

void frd_reader::parse_result_block(std::ifstream &stream) {
    char* buffer = new char[6];
    stream.read(buffer, 6);
    std::string name(buffer);
    delete[] buffer;

    buffer = new char[12];
    stream.read(buffer, 12);
    std::string value(buffer);
    delete[] buffer;

    buffer = new char[12];
    stream.read(buffer, 12);
    int nNodes = std::stoi(std::string(buffer));
    delete[] buffer;
}


void frd_reader::read(const char* frd_path) {
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

    while (ptr < mEnd && !kill) {
        // read buffer until next linebreak
        int line_length = get_line(ptr, mEnd);

        // If not enough characters to read a code...
        if (line_length < 6) {
            ptr += line_length;
            continue;
        }
        std::string code(ptr, 6);
        ltrim(code);
        rtrim(code);

        if (std::strcmp(code.c_str(), "9999") == 0) {
            break;
        } else if (std::strcmp(code.c_str(), "1C") == 0) {
            std::cout << "Reading model header..." << std::endl;
            ptr += line_length;
            read_header(ptr);
        } else if (std::strcmp(code.c_str(), "2C") == 0) { // nodes
            nNodes = std::stoi(std::string(ptr + 6, 30));
            std::cout << nNodes << " nodes found." << std::endl;

            int format = std::stoi(std::string(ptr + 36, 38));

            ptr += line_length;
            switch (format) {
                case(1):
                    read_nodes_ascii(ptr);
                    break;
                default:
                    read_nodes_binary(ptr);
                    break;
            }
        } else if (std::strcmp(code.c_str(), "3C") == 0) { // element block
            nElements = std::stoi(std::string(ptr + 6, 30));
            std::cout << nElements << " elements found." << std::endl;

            int format = std::stoi(std::string(ptr + 36, 38));

            ptr += line_length;
            switch (format) {
                case(1):
                    read_elements_ascii(ptr);
                    break;
                default:
                    read_elements_binary(ptr);
                    break;
            }
        } else if (std::strcmp(code.c_str(), "1P") == 0) { // step block
            int resultId = std::stoi(std::string(ptr + 10, 26));
            ptr += line_length;
            line_length = get_line(ptr, mEnd);

            // Now the code should be 100C
            code.assign(ptr, 6);
            ltrim(code);

            if (std::strcmp(code.c_str(), "100C") == 0) { // results block
                // Extract results header
                std::string setName(ptr + 6, 6);
                float value = std::stof(std::string(ptr + 12, 12));
                int nValues = std::stoi(std::string(ptr + 24, 12));

                if (nNodes != nValues) {
                    std::cout << "Number of values doesn't correspond to number of nodes...\nThis is very confusing. Breaking..." << std::endl;
                    break;
                }

                std::string text(ptr + 36, 20);
                int icType = std::stoi(std::string(ptr + 56, 2));
                int nStep = std::stoi(std::string(ptr + 58, 5));
                std::string analysis(ptr + 63, 10);
                int format = std::stoi(std::string(ptr + 73, 2));

                //std::cout << "icType: " << icType << ", nStep: " << nStep << ", analysis: " << analysis << ", format: " << format << std::endl;

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
                //std::cout << name << ", num components: " << numComponents << ", type: " << irType << std::endl;

                ptr += line_length;
                std::vector<std::string> componentNames;

                for (int i = 0; i < numComponents; ++i) {
                    line_length = get_line(ptr, mEnd);

                    std::string componentName(ptr + 5, 8);
                    ltrim(componentName);
                    rtrim(componentName);

                    componentNames.push_back(componentName);

                    int icMenu = std::stoi(std::string(ptr + 13, 5));
                    int icType = std::stoi(std::string(ptr + 18, 5));
                    int icIndex1 = std::stoi(std::string(ptr + 23, 5));
                    int icIndex2 = std::stoi(std::string(ptr + 28, 5));
                    if (line_length > 34) {
                        int iExist = std::stoi(std::string(ptr + 33, 5));

                        if (iExist > 0) numComponents--;
                        if (line_length >= 41) {
                            std::string extraName(ptr + 33, 8);
                            ltrim(extraName);
                        }
                    }
                    ptr += line_length;
                }

                std::vector<std::map<int, float>> values;

                for (int i = 0; i < numComponents; ++i) {
                    values.push_back(std::map<int, float>());
                }

                switch (format) {
                    case(1):
                        for (int i = 0; i < nNodes; ++i) {
                            int nodeId = std::stoi(std::string(ptr + 3, 10));

                            for (int j = 0; j < numComponents; ++j) {
                                values[j][nodeId] = std::stof(std::string(ptr + 13 + 12 * j, 12));
                            }

                            line_length = get_line(ptr, mEnd);
                            ptr += line_length;
                        }
                        break;
                    default:
                        for (int i = 0; i < nNodes; ++i) {
                            int nodeId = *(int*)ptr;
                            ptr += sizeof(int);
                            for (int j = 0; j < numComponents; ++j) {
                                float v = *(float*)ptr;
                                values[j][nodeId] = v;
                                ptr += sizeof(float);
                            }
                        }
                        break;
                }

                for (int i = 0; i < numComponents; ++i) {
                    mValues[nStep][name][componentNames[i]] = values[i];
                }
            }
        } else {
            std::cout << "Unrecognized code encountered: '" << code << "'" << std::endl;
            break;
        }
    }

    delete[] buffer;

    std::cout << "Done." << std::endl;
}

void frd_reader::read_header(char* &ptr) {
    std::string code;
    code.assign(ptr, 6);
    ltrim(code);

    std::cout << "read_header()" << std::endl;

    while (std::strcmp(code.c_str(), "1U") == 0) {
        int line_length = get_line(ptr, mEnd);
        std::string header_data(ptr + 6, 66);

        ptr += line_length;

        code.assign(ptr, 6);
        ltrim(code);

        std::cout << header_data << std::endl;
    }
}

void frd_reader::read_nodes_ascii(char* &ptr) {
    for (size_t i = 0; i < nNodes; ++i) {
        int line_length = get_line(ptr, mEnd);
        int id = std::stoi(std::string(ptr + 3, 10));
        double x = std::stod(std::string(ptr + 13, 12));
        double y = std::stod(std::string(ptr + 25, 12));
        double z = std::stod(std::string(ptr + 37, 12));

        mNodes[id] = frd_node(id, x, y, z);
        ptr += line_length;
    }
}

void frd_reader::read_nodes_binary(char* &ptr) {
    frd_node* nodes_ptr = (frd_node*)ptr;
    for (size_t i = 0; i < nNodes; ++i) {
        mNodes[nodes_ptr[i].id] = nodes_ptr[i];
    }

    ptr += nNodes * sizeof(frd_node);
    std::cout << "Finished reading nodes." << std::endl;
}

void frd_reader::read_elements_ascii(char* &ptr) {
    int nIndices;
    for (size_t i = 0; i < nElements; ++i) {
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
        for (int i = 0; i < nIndices; ++i) {
            element.indices[i] = std::stoi(std::string(ptr + 3 + 10 * i, 10));
        }

        mElements[header.id] = element;

        ptr += line_length;
    }
}

void frd_reader::read_elements_binary(char* &ptr) {
    int nIndices;
    for (int i = 0; i < nElements; ++i) {
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

void frd_reader::read_results_ascii(char* &ptr, int nComponents) {
    for (int i = 0; i < nNodes; ++i) {
        int line_length = get_line(ptr, mEnd);
        std::string line(ptr, line_length);
        rtrim(line);

        ptr += line_length;
    }
}

void frd_reader::read_results_binary(char* &ptr, int nComponents) {
    for (int i = 0; i < nNodes; ++i) {
        int nodeId = *(int*)ptr;
        ptr += sizeof(int);
        for (int j = 0; j < nComponents; ++j) {
            float v = *(float*)ptr;
            ptr += sizeof(float);
        }
    }
}
