#!/bin/bash

# Define directories
BUILD_DIR="build/pyfrd"
PACKAGE_DIR="pyfrd_package"

# Clean previous package builds
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/pyfrd"

# Copy the shared object file (.so) to the package directory
LIBFILE=$(find "$BUILD_DIR" -name "*.so")
cp "$LIBFILE" "$PACKAGE_DIR/pyfrd/"

# Create an empty __init__.py file and add necessary imports
echo 'from .pyfrd import FrdReader, test' > "$PACKAGE_DIR/pyfrd/__init__.py"

# Create pyproject.toml (modern packaging)
cat <<EOL > "$PACKAGE_DIR/pyproject.toml"
[build-system]
requires = ["setuptools", "wheel"]
build-backend = "setuptools.build_meta"

[project]
name = "pyfrd"
version = "0.1"
description = "A Python interface for reading .frd files"
authors = [{ name = "T. Svilans & J. Huber" }]
readme = "README.md"
requires-python = ">=3.7"
dependencies = []

[tool.setuptools]
packages = ["pyfrd"]

[tool.setuptools.package-data]
pyfrd = ["*.so"]
EOL

# Create a README.md file
echo "# PyFRD" > "$PACKAGE_DIR/README.md"
echo "This is a Python package for reading .frd files." >> "$PACKAGE_DIR/README.md"

# Build the wheel
cd "$PACKAGE_DIR"
python -m pip install --upgrade build
python -m build

# Install the built package
pip install --force-reinstall dist/pyfrd-0.1-py3-none-any.whl

# Return to the original directory
cd ..

echo "Installation complete. Verify by running: python -c 'import pyfrd; pyfrd.test(1, 2)'"
