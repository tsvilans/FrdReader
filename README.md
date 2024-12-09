# FrdReader
A library for reading both binary and ASCII `.frd` results files from CalculiX.
Includes .NET/CLI and Python wrappers.

![Results of a laminated beam.](img/frd_reader_01.png)

## Installation

- Clone the repository.

For `libfrd`, in the repository directory:
```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

For the `FrdReader` .NET assembly:
- Build the project in the `FrdReader` subdirectory.

For `pyfrd`:
- In the `pyfrd` subdirectory, run `pip install .`

This will build and install `pyfrd` and its dependencies using the `scikit-build-core` build system.

## Dependencies
- CMake v3.15 or higher

Additionally, for `pyfrd`:
- Python
- sci-kit-build (`pip install scikit-build`)
