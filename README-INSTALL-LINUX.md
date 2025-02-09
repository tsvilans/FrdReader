### README-INSTALL-LINUX.md

# FrdReader Installation Guide for Linux

This guide will walk you through the necessary steps to build the FrdReader package and install the Python module on a Linux system.

## Prerequisites

Ensure you have the following installed:
- CMake
- GCC or another C++ compiler
- Python 3.x
- `pip`
- `setuptools` and `wheel` Python packages

You can install the prerequisites using:
```bash
sudo apt-get update
sudo apt-get install cmake g++ python3 python3-pip
pip install setuptools wheel
```

## Building the Package

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/tsvilans/FrdReader.git
   cd FrdReader
   ```

2. **Create a Build Directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Run CMake**:
   ```bash
   cmake ..
   ```

4. **Compile the Project**:
   ```bash
   make
   ```

## Installing the Python Package

1. **Navigate to the Project Root**:
   ```bash
   cd ..
   ```

2. **Run the Install Script**:
   Ensure the `install_pyfrd.sh` script is executable and run it:
   ```bash
   chmod +x install_python_package.sh
   ./install_python_package.sh
   ```

## Verify Installation

Open a Python shell and verify the installation:
```python
import pyfrd
pyfrd.test(1, 2)
```

You should see the message "Hello from FrdReader!".

## Notes

- Ensure all dependencies are properly installed.
- If you encounter any issues, check the build logs and ensure the paths are correctly set.

This guide helps you set up the FrdReader package on a Linux system, compile the necessary components, and install the Python module. For more details, refer to the project documentation or contact the maintainers.