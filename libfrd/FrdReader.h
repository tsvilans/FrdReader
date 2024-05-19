#ifndef FRD_READER_H
#define FRD_READER_H

#include "util.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#pragma pack (1)
struct frd_node
{
	int id;
	double x, y, z;

	frd_node() : id(-1), x(0), y(0), z(0) { }

	frd_node(int _id, double _x, double _y, double _z) : id(_id), x(_x), y(_y), z(_z) { }
};
#pragma pack (0)

#pragma pack (1)
struct frd_element_header
{
	int id, type, group, material;
};
#pragma pack (0)

struct frd_element
{
	frd_element_header header;
	std::vector<int> indices;
};

struct frd_results_block
{
	std::string name;
	int format, numComponents, ictype, nstep;
};

class FrdReader
{
public:
	int get_line(char* ptr, char* end);
	void parse_result_block(std::ifstream stream);

	void read(const char* frd_path);

	void read_header(char* &ptr);

	void read_nodes_ascii(char* &ptr);
	void read_nodes_binary(char* &ptr);

	void read_elements_ascii(char* &ptr);
	void read_elements_binary(char* &ptr);

	void read_results_ascii(char* &ptr, int nComponents);
	void read_results_binary(char* &ptr, int nComponents);

	inline size_t num_nodes() { return mNodes.size(); }

	std::map<size_t, frd_node> mNodes;
	std::map<size_t, frd_element> mElements;

	std::map<std::string, std::map<std::string, std::map<int, float>>> mValues;
	std::map<std::string, frd_results_block> mMetadata;

private:
	char* mEnd;
	size_t nNodes, nElements;

};

#endif


