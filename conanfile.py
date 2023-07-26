from conans import CMake, ConanFile


class ExampleApp(ConanFile):
    name = "ExampleApp"
    version = "1.0"
    description = "Example App"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "cmake"
    exports_sources = "source/*"
    requires = [
        "boost/1.82.0@",
        "nlohmann_json/3.11.2",
        "openssl/1.1.1h",
        "zlib/1.2.13",
        "bzip2/1.0.8",
        "libbacktrace/cci.20210118",
    ]

    def requirements(self):
        if self.settings.compiler != "apple-clang":
            self.requires("tbb/2020.3")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="source")

        cmake.build()

        # Explicit way:
        # self.run('cmake "%s/src" %s' % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.hpp", dst="include", src="src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["me"]
        self.cpp_info.defines = ["BITWYRE_WITH_MYSQL=0"]
        self.cpp_info.defines = ["BUILD_ME_CLI=1"]
