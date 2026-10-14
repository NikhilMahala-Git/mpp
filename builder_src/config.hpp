#ifndef _BUILDER_CONFIG_HPP_
#define _BUILDER_CONFIG_HPP_
#include <filesystem>
#include <string_view>

const std::filesystem::path Project_Directory              = "./";

// Build and Install Directories

const std::filesystem::path Project_Build_Directory            = Project_Directory / "build";
const std::filesystem::path Project_Install_Directory          = Project_Directory / "install";

// Cpp source directories

const std::filesystem::path Project_Src_Directory              = Project_Directory / "src";
const std::filesystem::path Project_Mpp_Lib_Directory          = Project_Directory / "mpp_lib";
const std::filesystem::path Project_Platform_Directory         = Project_Directory / "platform";

const std::filesystem::path Project_Native_Platform_Directory  = Project_Platform_Directory / "native_impl";
const std::filesystem::path Project_Web_Platform_Directory     = Project_Platform_Directory / "web_impl";

const std::filesystem::path Project_Web_Platform_Ts_Entry_File = Project_Web_Platform_Directory / "index.ts";

#ifdef USE_NIKHIL_CONFIG
const std::string_view Native_Compiler = "/home/nikhil/opt/llvm/bin/clang++"; // Compiler used for native build(s)
const std::string_view Llvm_Ar_Linker = "/home/nikhil/opt/llvm/bin/llvm-ar";  // Linker used for native build linking
const std::string_view Clang_Plus_Plus_Compiler = Native_Compiler;            // Compiler used for web build(s)

const std::string_view Npm = "/home/nikhil/opt/node-v26.10.0-linux-x64/bin/npm"; // Npm for installing esbuild
const std::string_view Npx = "/home/nikhil/opt/node-v26.10.0-linux-x64/bin/npx"; // Npx for executing esbuild cli
#else
const std::string_view Native_Compiler = "clang++";           // Compiler used for native build(s)
const std::string_view Llvm_Ar_Linker = "llvm-ar" ;           // Linker used for native build linking
const std::string_view Clang_Plus_Plus_Compiler = "clang++";  // Compiler used for web build(s)

const std::string_view Npm = "npm"; // Npm for installing esbuild
const std::string_view Npx = "npx"; // Npx for executing esbuild cli
#endif

const std::filesystem::path Project_Raylib_Include_Directory   = "/home/nikhil/opt/raylib/include/";

#endif // _BUILDER_CONFIG_HPP_

