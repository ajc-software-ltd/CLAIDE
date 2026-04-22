# CLAIDE AI Content Creator
**Classification**: Runtime Reference Book
A cross-platform C++23 AIO IDE for code, media creation, and AI-powered content generation.
**AJC-Software Ltd © 2026**

---

*Table of Contents*

* [1. Programming Languages](#1-programming-languages)
* [2. OS References](#2-os-references)
* [3. Software Libraries](#3-software-libraries)

[Back to Top](#agents-coding-refs)

---

## 1. PROGRAMMING LANGUAGES

⚠️ **VERIFICATION NOTICE**: All resources below verified as FREE FOR COMMERCIAL USE

### C++ Standard & Resources

- **ISO C++ Foundation**: isocpp.org
  C++ authority - standardization, articles, guidelines, updates (C++23+), conformance

- **C++ Core Guidelines**: isocpp.github.io/CppCoreGuidelines
  C++ guidelines - Herb Sutter & Andrei Alexandrescu - patterns, abstractions, RAII

- **cppreference.com**: en.cppreference.com/w/cpp
  C++ reference - std library, operators, templates, types


### Build Requirements

#### CachyOS / Linux x64
```bash
# Install system packages
sudo pacman -S wxgtk wxgtk3 wxgtk3-libraries wxgtk3-gtk3 wxgtk3-gtk3-libraries \
    spdlog imagemagick vulkan-icd-loader vulkan-validationlayers
```

#### Windows x64
```bash
# vcpkg
./vcpkg integrate install
vcpkg install wxwidgets spdlog imagemagick
```

---
**DIRECTIVE**: All references above verified as free for commercial use.

---

## 2. OS REFERENCES

⚠️ **VERIFICATION NOTICE**: All resources below verified as FREE FOR COMMERCIAL USE

### Linux & Unix Ecosystem

- **Linux Kernel Docs**: https://docs.kernel.org/
  Linux kernel docs - APIs, subsystems, interfaces, architecture

- **POSIX Standards**: certics.com/standards-posix
  POSIX.1-2017 & IEEE 1003.1 - Unix APIs, shell, filesystems

---

## 3. SOFTWARE LIBRARIES

⚠️ **VERIFICATION NOTICE**: All resources below verified as FREE FOR COMMERCIAL USE

### Graphics & Compute APIs

- **Vulkan SDK**: https://vulkan.lunarg.com/
  Low-level GPU API (optional, via VulkanAI library)

### GUI Framework

- **wxWidgets 3.2+**: https://docs.wxwidgets.org/
  Cross-platform C++ GUI toolkit - widgets, dialogs, AUI docking
  Components: core, base, aui, image

### Logging

- **spdlog 1.x**: github.com/gabime/spdlog
  Fast C++ logging library with rotating file sink

### Image Processing

- **ImageMagick (Magick++)**: https://imagemagick.org/
  Image manipulation and metadata extraction

### GPU Rendering

- **Vulkan SDK**: https://vulkan.lunarg.com/
  Low-level GPU API (optional, via VulkanAI library)

### Testing

- **Catch2 v3**: github.com/catchorg/Catch2
  Modern C++ test framework

### Build & Deployment Tools 

- **CMake**: cmake.org
  Cross-platform build system generator - CMakeLists.txt, toolchains, packaging

### Python Libraries

- **Python 3.13**: https://docs.python.org/3.13/
  Python 3.13.13 - Official documentation, standard library, tutorials

- **PyTorch**: https://pytorch.org/docs/
  PyTorch 2.x - Deep learning framework, neural networks, GPU compute

- **TensorFlow**: https://www.tensorflow.org/api_docs
  TensorFlow 2.x - Deep learning, neural networks, production ML

- **scikit-learn**: https://scikit-learn.org/stable/
  scikit-learn 1.x - Machine learning, data mining, statistical models

- **Pandas**: https://pandas.pydata.org/docs/
  Pandas 2.x - Data manipulation, analysis, structured data

- **NumPy**: https://numpy.org/doc/
  NumPy 2.x - Numerical computing, arrays, linear algebra

- **Matplotlib**: https://matplotlib.org/stable/
  Matplotlib 3.x - Data visualization, plotting, graphs

- **Requests**: https://docs.python-requests.org/
  Requests 2.x - HTTP library, REST API client

- **FastAPI**: https://fastapi.tiangolo.com/
  FastAPI 0.x - Modern web framework, async APIs, high performance


### Build Systems & Toolchains

- **CMake**: https://cmake.org/
  Cross-platform build system generator - CMakeLists.txt, toolchains, packaging

- **vcpkg**: github.com/Microsoft/vcpkg
  C++ library manager - wxWidgets, spdlog, ImageMagick integration

### Testing Frameworks

- **Catch2 v3**: github.com/catchorg/Catch2
  Modern C++ test framework - BDD style, reporting, benchmarks

### Python Bindings (Planned)

- **pybind11**: github.com/pybind/pybind11
  C++11 Python bindings for IDE automation

---
**DIRECTIVE**: All references above verified as free for commercial use.
