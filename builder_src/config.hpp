#ifndef _BUILDER_CONFIG_HPP_
#define _BUILDER_CONFIG_HPP_
#include <filesystem>
#include <string_view>

const std::filesystem::path Project_Directory              = "./";

// Build and Install Directories

const std::filesystem::path Project_Build_Directory        = Project_Directory / "build";
const std::filesystem::path Project_Install_Directory      = Project_Directory / "install";

// Cpp source directories

const std::filesystem::path Project_Src_Directory          = Project_Directory / "src";
const std::filesystem::path Project_Mpp_Lib_Directory      = Project_Directory / "mpp_lib";
const std::filesystem::path Project_Web_Mpp_Impl_Directory = Project_Directory / "web_mpp_impl";

const std::string_view Builder_Native_Compiler = "g++";
const std::string_view Llvm_Ar_Linker = "/home/nikhil/opt/llvm/bin/llvm-ar";

#endif // _BUILDER_CONFIG_HPP_

