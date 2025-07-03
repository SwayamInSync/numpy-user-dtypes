# Numpy-QuadDType

A cross-platform Quad (128-bit) float Data-Type for NumPy.

## Installation

```bash
pip install numpy
pip install numpy-quaddtype
```

## Usage

```python
import numpy as np
from numpy_quaddtype import QuadPrecDType, QuadPrecision

# using sleef backend (default)
np.array([1,2,3], dtype=QuadPrecDType())
np.array([1,2,3], dtype=QuadPrecDType("sleef"))

# using longdouble backend
np.array([1,2,3], dtype=QuadPrecDType("longdouble"))
```

## Installation from source

The code needs the quad precision pieces of the sleef library, which
is not available on most systems by default, so we have to generate
that first.  The below assumes one has the required pieces to build
sleef (cmake and libmpfr-dev), and that one is in the package
directory locally.

```bash
git clone --branch 3.8 https://github.com/shibatch/sleef.git
cd sleef
cmake -S . -B build -DSLEEF_BUILD_QUAD:BOOL=ON -DSLEEF_BUILD_SHARED_LIBS:BOOL=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
cmake --build build/ --clean-first -j
cd ..
```

Building the `numpy-quaddtype` package from locally installed sleef:
```bash
export SLEEF_DIR=$PWD/sleef/build
export LIBRARY_PATH=$SLEEF_DIR/lib
export C_INCLUDE_PATH=$SLEEF_DIR/include
export CPLUS_INCLUDE_PATH=$SLEEF_DIR/include

# Install the package
pip install meson-python numpy pytest
pip install -e . -v --no-build-isolation
export LD_LIBRARY_PATH=$SLEEF_DIR/lib
```

