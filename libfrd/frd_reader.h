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



class frd_reader
{
public:

	std::map<int, int> ELEMENT_TYPE_MAP{
		{ 1, 8 },
		{ 2, 6 },
		{ 3, 4 },
		{ 4, 20 },
		{ 5, 15 },
		{ 6, 10 },
		{ 7, 3 },
		{ 8, 6 },
		{ 9, 4 },
		{ 10, 8 },
		{ 11, 2 },
		{ 12, 3 },
	};

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

	std::map<int, std::map<std::string, std::map<std::string, std::map<int, float>>>> mValues;
	//std::map<int, std::map<std::string, std::map<std::string, std::vector<float>>>> mValues;
	std::map<int, std::map<std::string, frd_results_block>> mMetadata;

private:
	char* mEnd;
	size_t nNodes, nElements;

};

template<typename frd_type>
class frd
{
	const std::map<int, std::map<std::string, std::map<std::string, frd_type*>>> values;

	struct step_iterator
	{
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = frd_type;
		using pointer = frd_type*;
		using reference = frd_type&;

		step_iterator(pointer ptr, size_t step_stride) : m_ptr(ptr), m_stride(step_stride) {}

		const reference operator*() const { return *m_ptr; }
		const pointer operator->() { return m_ptr; }

		step_iterator& operator++() { m_ptr+= m_stride; return *this; }

		step_iterator& operator++(frd_type) { step_iterator tmp = *this; ++(*this); return tmp; }

		friend bool operator == (const step_iterator& a, const step_iterator& b) { return a.m_ptr == b.m_ptr; };
		friend bool operator != (const step_iterator& a, const step_iterator& b) { return a.m_ptr != b.m_ptr; };

	private:
		pointer m_ptr;
		size_t m_stride;
	};

	step_iterator begin() { return step_iterator(&m_data[0], m_stride_step); }
	step_iterator end() { return step_iterator(&m_data[m_data_size - m_stride_step]); }

private:
	frd_type* m_data;

	size_t m_data_size;
	size_t m_stride_step;
	size_t* m_stride_field; // each field can be a different size due to different number of components
	size_t m_stride_component;
};

class frd_reader2
{
public:

	std::map<int, int> ELEMENT_TYPE_MAP{
		{ 1, 8 },
		{ 2, 6 },
		{ 3, 4 },
		{ 4, 20 },
		{ 5, 15 },
		{ 6, 10 },
		{ 7, 3 },
		{ 8, 6 },
		{ 9, 4 },
		{ 10, 8 },
		{ 11, 2 },
		{ 12, 3 },
	};

	int get_line(char* ptr, char* end);
	void parse_result_block(std::ifstream stream);

	void read(const char* frd_path);

	void read_header(char*& ptr);

	void read_nodes_ascii(char*& ptr);
	void read_nodes_binary(char*& ptr);

	void read_elements_ascii(char*& ptr);
	void read_elements_binary(char*& ptr);

	void read_results_ascii(char*& ptr, int nComponents);
	void read_results_binary(char*& ptr, int nComponents);

	//inline size_t num_nodes() { return mNodes.size(); }

	int get(size_t step_id, std::string field, std::string component, size_t node_id, double &result);
	int get(size_t step_id, std::string field, std::string component, std::vector<size_t> node_ids, std::vector<double> results);

	std::map<size_t, frd_node> mNodes;
	std::map<size_t, frd_element> mElements;

	std::map<int, std::map<std::string, std::map<std::string, std::map<int, float>>>> mValues;
	std::map<int, std::map<std::string, frd_results_block>> mMetadata;

	int num_nodes;
	int num_elements;

	std::vector<size_t> step_ids;
	std::vector<std::string> field_names;
	std::vector<std::string> component_names;

	std::vector<size_t> step_offsets;
	std::vector<size_t> field_offsets;
	std::vector<size_t> component_offsets;

	double* data;

	size_t row_step;


private:
	char* mEnd;

};

#endif


