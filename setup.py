"""
@file setup.py
@brief Python package configuration for SCServo library

@details This file configures the Python bindings for the Feetech SCServo library.
The actual Python extension module is built by CMake using nanobind, which
provides efficient C++ to Python bindings with minimal overhead.

@section build_process Build Process
The Python module (scservo_python) is compiled by the CMakeLists.txt
configuration, not by this setup.py file. CMake handles:
- Compiling C++ source files
- Linking with nanobind
- Creating the Python extension module (.so file)

@section installation Installation
To build and install the Python bindings:

@code{.sh}
$ mkdir build && cd build
$ cmake ..
$ make -j4
$ pip install -e .  # Development install
@endcode

Or use the compiled module directly without installation by adding
the build directory to PYTHONPATH.

@section usage Usage Example
@code{.py}
import scservo_python

servo = scservo_python.SMS_STS()
servo.begin(1000000, "/dev/ttyUSB0")
servo.InitMotor(1, 0, 1)  # ID=1, servo mode, enable torque
servo.WritePosEx(1, 2048, 1000, 50)
@endcode

@section module_contents Module Contents
The scservo_python module provides Python bindings for:
- SMS_STS: SMS/STS series servo control
- SCSCL: SCSCL series servo control
- HLSCL: HLSCL/HLS series servo control

@section dependencies Dependencies
- nanobind: Modern C++ to Python binding library
- SCServo library: C++ implementation (linked statically)

@see src/bindings.cpp for Python binding implementation
@see CMakeLists.txt for build configuration
"""

from setuptools import setup

setup(
    name='scservo_python',
    version='0.1',
    description='Python bindings for SCServo using nanobind',
    py_modules=[],
    packages=[],
)
