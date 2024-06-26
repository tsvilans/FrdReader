#ifndef PY_FRD_READER_H
#define PY_FRD_READER_H
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
// #include <format>
#include <sstream> // Replace <format> with <sstream>
#include <iomanip> // Include this header for setprecision


#include "frd_reader.h"

namespace py = pybind11;

void exportFrdReader(py::module_ m);

#endif // PY_FRD_READER_H