#!/bin/bash

# Define the existing build directory
BUILD_DIR="build/pyfrd"
PACKAGE_DIR="pyfrd_package"

# Clean previous package builds
if [ -d "$PACKAGE_DIR" ]; then
    rm -rf "$PACKAGE_DIR"
fi

# Create necessary directories
mkdir -p "$PACKAGE_DIR/pyfrd"

# Copy the shared object file to the package directory
cp "$BUILD_DIR/pyfrd.cpython-311-x86_64-linux-gnu.so" "$PACKAGE_DIR/pyfrd/"

# Create an empty __init__.py file and add the necessary imports
echo 'from .pyfrd import FrdReader, test' > "$PACKAGE_DIR/pyfrd/__init__.py"

# Create setup.py for the package
cat <<EOL > "$PACKAGE_DIR/setup.py"
from setuptools import setup, find_packages

setup(
    name='pyfrd',
    version='0.1',
    packages=find_packages(),
    package_data={
        'pyfrd': ['*.so'],
    },
)
EOL

# Build the wheel
cd "$PACKAGE_DIR"
python setup.py bdist_wheel

# Install the wheel using pip
python -m pip install --force-reinstall dist/pyfrd-0.1-py3-none-any.whl

# Return to the original directory
cd ..

echo "Installation complete. Verify by running: import pyfrd; pyfrd.test(1, 2)"
