# Copyright (C) 2026 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>
# SPDX-License-Identifier: MIT

from conan import ConanFile
from conan.tools.build import can_run
from conan.tools.cmake import CMake, cmake_layout


class QtLoggerTestPackageConan(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires(self.tested_reference_str)

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self):
        if can_run(self):
            self.run(self.cpp.build.bindirs[0] + "/test_package", env="conanrun")
