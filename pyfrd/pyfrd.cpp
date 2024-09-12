#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include "pyFrdReader.h"

namespace py = pybind11;

#ifdef _WIN32
#include "framework.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#else
// Linux-specific code can be added here if needed
#endif

void test(int a, int b) {
    std::cout << "Hello from FrdReader!" << std::endl;
}

PYBIND11_MODULE(pyfrd, m) {
    // Don't auto-generate ugly, C++-style function signatures.
    py::options docOptions;
    docOptions.disable_function_signatures();
    docOptions.enable_user_defined_docstrings();

    m.doc() = "PyFrd module for reading results files from CalculiX."; // optional module docstring

    // Export the python bindings.
    exportFrdReader(m);

    m.def("test", &test, "test(int, int) -> None\n\n"
          "Display a welcome message.",
          py::arg("argument0"), py::arg("argument1"));
} // PYBIND11_MODULE
