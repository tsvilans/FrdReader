#include "pyFrdReader.h"

void exportFrdReader(py::module_ m)
{
    py::class_<frd_node>(m, "FrdNode")
        .def(py::init<>())
        .def_readonly("id", &frd_node::id)
        .def_readonly("x", &frd_node::x)
        .def_readonly("y", &frd_node::y)
        .def_readonly("z", &frd_node::z)
        .def("__repr__",
            [](const frd_node& node) {
                std::ostringstream oss;
                oss << "FrdNode (" << node.id << ", " << std::fixed << std::setprecision(3) << node.x << " " << node.y << " " << node.z << ")";
                return oss.str();
            }
        );

    py::class_<frd_element>(m, "FrdElement")
        .def(py::init<>())
        .def_property_readonly("id", [](frd_element const& e) { return e.header.id; })
        .def_property_readonly("type", [](frd_element const& e) { return e.header.type; })
        .def_property_readonly("group", [](frd_element const& e) { return e.header.group; })
        .def_property_readonly("material", [](frd_element const& e) { return e.header.material; })
        .def_readonly("indices", &frd_element::indices)
        .def("__repr__",
            [](const frd_element& element) {
                std::ostringstream oss;
                oss << "FrdElement (" << element.header.id << ", " << element.indices.size() << " nodes)";
                return oss.str();
            }
        )

        .def_readonly("indices", &frd_element::indices);

    py::class_<frd_results_block>(m, "FrdResultsHeader")
        .def(py::init<>())
        .def_property_readonly("name", [](const frd_results_block& header) { return header.name; })
        .def_property_readonly("format", [](const frd_results_block& header) { return header.format; })
        .def_property_readonly("ictype", [](const frd_results_block& header) { return header.ictype; })
        .def_property_readonly("num_components", [](const frd_results_block& header) { return header.numComponents; })
        .def_property_readonly("num_step", [](const frd_results_block& header) { return header.nstep; })
        .def("__repr__",
            [](const frd_results_block& header) {
                std::ostringstream oss;
                oss << "FrdResultsHeader (" << header.name << ")";
                return oss.str();
            }
        );

    py::bind_map<std::map<int, frd_node>>(m, "FrdNodeMap");
    py::bind_map<std::map<int, frd_element>>(m, "FrdElementMap");
    //py::bind_map<std::map<std::string, frd_results_block>>(m, "FrdResultsHeaderMap");

    py::class_<frd_reader>(m, "FrdReader")
        .def(py::init<>())
        .def("read", &frd_reader::read,
            "read(string) -> None\n\n"
            "Display a welcome message.",
            py::arg("frd_path"))
        .def_property_readonly("num_nodes", &frd_reader::num_nodes,
            "num_nodes() -> int\n\n"
            "Get number of nodes.")
        .def_readonly("nodes", &frd_reader::mNodes,
            "nodes\n\n"
            "Node data.")
        .def_readonly("elements", &frd_reader::mElements,
            "elements\n\n"
            "Element data.")
        .def_readonly("values", &frd_reader::mValues,
            "values\n\n"
            "Result data.");
}
