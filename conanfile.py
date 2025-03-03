import os
from conan import ConanFile
from conan.tools.cmake import CMakeDeps
from conan.tools.cmake import CMake, CMakeToolchain

class DsExpConanPackage(ConanFile):
    name = "ds_exp"
    version = "0.1"
    license = "MIT"
    author = "Francesco Fucci"
    description = "Package with some experiements with interesting data structures"
    settings = "os", "compiler", "build_type", "arch"

    def generate(self):
        self.output.info("Generating the package")
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self, generator="Ninja")
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["ds_exp"]

    def requirements(self):
        self.requires("ninja/1.10.2")
        self.requires("benchmark/1.9.0")
        self.requires("boost/1.86.0")
        self.requires("spdlog/1.8.5")
        self.requires("gtest/1.15.0")