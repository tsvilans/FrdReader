#include "frd_reader.h"
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>

static void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

int main(int argc, char** argv)
{
    require(argc == 2, "Pass a temporary FRD filename");
    const char* path = argv[1];
    {
        std::ofstream file(path);
        file << "    2C" << std::setw(30) << 2 << std::setw(2) << 1 << std::string(36, ' ') << '\n';
        for (int id : {7, 19}) {
            file << " -1" << std::setw(10) << id << std::scientific << std::setprecision(5)
                 << std::setw(12) << 1.0 << std::setw(12) << 2.0 << std::setw(12) << 3.0 << '\n';
        }
        file << " -3\n  9999\n";
    }
    frd_reader2 reader;
    reader.read(path);
    require(reader.mNodes.size() == 2, "Read must reach the mesh instead of stalling in its scan");
    require(reader.mNodes.at(19).z == 3.0, "ASCII node coordinates must be parsed");

    // Requests deliberately use the second step, field and component. The old
    // unconditional breaks only inspected the first entry in each index.
    reader.step_ids = {10, 20};
    reader.step_offsets = {1, 2};
    reader.field_names = {"DISP", "STRESS"};
    reader.field_offsets = {1, 2};
    reader.component_names = {"D1", "D2"};
    reader.component_offsets = {1, 2};
    reader.num_nodes = 3;
    reader.row_step = 8;
    double rows[24] = {};
    rows[0] = 7; rows[6] = 107;
    rows[8] = 3; rows[14] = 103;
    rows[16] = 19; rows[22] = 119;
    reader.data = rows;
    double result = -1;
    require(reader.get(20, "STRESS", "D2", 3, result) == 0 && result == 103,
            "Scalar lookup must resolve entries beyond the first index");
    require(reader.get(99, "STRESS", "D2", 3, result) == -1, "Missing steps must fail");
    require(reader.get(20, "UNKNOWN", "D2", 3, result) == -1, "Missing fields must fail");
    require(reader.get(20, "STRESS", "UNKNOWN", 3, result) == -1, "Missing components must fail");
    require(reader.get(20, "STRESS", "D2", 99, result) == -1, "Missing nodes must fail");
    std::vector<double> results = {-1};
    require(reader.get(20, "STRESS", "D2", std::vector<size_t>{19, 7, 3}, results) == 0,
            "Batch lookup must accept out-of-order IDs");
    require(results == std::vector<double>({-1, 119, 107, 103}), "Batch results must reach the caller in request order");
    const auto previous = results;
    require(reader.get(20, "STRESS", "D2", std::vector<size_t>{7, 99}, results) == -1,
            "A missing batch node must fail");
    require(results == previous, "Failed batch lookup must not append partial results");
    require(reader.get(20, "STRESS", "D2", std::vector<size_t>{}, results) == 0,
            "Empty batch must succeed");

    // The parser now uses bounded, automatically managed buffers and a stream reference.
    {
        std::ofstream file(path);
        file << "DISP  " << std::setw(12) << 1.0 << std::setw(12) << 2;
    }
    std::ifstream stream(path);
    reader.parse_result_block(stream);
    require(stream.tellg() == 30, "Result-header parser must consume the caller's stream");
    stream.close();

    // Unsupported records must stop parsing instead of leaving the pointer unchanged.
    {
        std::ofstream file(path);
        file << "ABCDEF\n  9999\n";
    }
    reader.read(path);
    std::remove(path);
    std::cout << "frd_reader2 regression tests passed." << std::endl;
}
