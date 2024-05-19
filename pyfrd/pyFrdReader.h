#ifndef PY_FRD_READER_H
#define PY_FRD_READER_H
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
#include <format>

#include "FrdReader.h"

namespace py = pybind11;

void exportFrdReader(py::module_ m);

#endif