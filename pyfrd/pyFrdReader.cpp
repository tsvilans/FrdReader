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
                return std::format("FrdNode ({}, {:.3f} {:.3f} {:.3f})", node.id, node.x, node.y, node.z);
            }
        )
        ;

    py::class_<frd_element>(m, "FrdElement")
        .def(py::init<>())
        .def_property_readonly("id", [](frd_element const& e) { return e.header.id; })
        .def_property_readonly("type", [](frd_element const& e) { return e.header.type; })
        .def_property_readonly("group", [](frd_element const& e) { return e.header.group; })
        .def_property_readonly("material", [](frd_element const& e) { return e.header.material; })
        .def("__repr__",
            [](const frd_element& element) {
                return std::format("FrdElement ({}, {} nodes)", element.header.id, element.indices.size());
            }
        )
        .def_readonly("indices", &frd_element::indices)
        ;

    py::class_<frd_results_block>(m, "FrdResultsHeader")
        .def(py::init<>())
        .def_property_readonly("name", &frd_results_block::name)
        .def_property_readonly("format", &frd_results_block::format)
        .def_property_readonly("ictype", &frd_results_block::ictype)
        .def_property_readonly("num_components", &frd_results_block::numComponents)
        .def_property_readonly("num_step", &frd_results_block::nstep)
        .def("__repr__",
            [](const frd_results_block& header) {
                return std::format("FrdResultsHeader ({})", header.name);
            }
        )
        .def_readonly("indices", &frd_element::indices)
        ;

    py::bind_map<std::map<int, frd_node>>(m, "FrdNodeMap");
    py::bind_map<std::map<int, frd_element>>(m, "FrdElementMap");

    py::class_<FrdReader>(m, "FrdReader")
        .def(py::init<>())
        .def("read", &FrdReader::read,
            "read(string) -> None\n\n"
            "Display a welcome message.",
            py::arg("frd_path"))
        .def_property_readonly("num_nodes", &FrdReader::num_nodes,
            "num_nodes() -> int\n\n"
            "Get number of nodes.")
        .def_readonly("nodes", &FrdReader::mNodes,
            "nodes\n\n"
            "Node data.")
        .def_readonly("elements", &FrdReader::mElements,
            "elements\n\n"
            "Element data.")
        .def_readonly("values", &FrdReader::mValues,
            "values\n\n"
            "Result data.")
        ;
}