#define NOB_IMPLEMENTATION
#include "./utility.hpp"
#include "./config.hpp"

bool nob_clean_dir_build(const std::filesystem::path &path) {
    if (!Builder::Fs::remove(path, true)) {
        return false;
    }
    return nob_mkdir_if_not_exists(path.c_str());
}

bool verify_web_build_dependencies() {
    if (Builder::Fs::find_executable(std::string(Clang_Plus_Plus_Compiler)) == std::filesystem::path {}){
        Builder::log_fmt(Builder::LogLevel_E::ERROR, 
                "Could not locate clang++ compiler: '{}'\n"
                "Make sure it is either present in PATH env-var or is path to an executable.\n"
                "Defined in builder_src/config.hpp as 'Clang_Plus_Plus_Compiler'\n\n"
                "Change it and try to recompile the builder :)"
                ,
                Clang_Plus_Plus_Compiler);
        return false;
    }

    if (Builder::Fs::find_executable(std::string(Npm)) == std::filesystem::path {}){
        Builder::log_fmt(Builder::LogLevel_E::ERROR, 
                "Could not locate npm: '{}'\n"
                "Make sure it is either present in PATH env-var or is path to an executable.\n"
                "Defined in builder_src/config.hpp as 'Npm'\n\n"
                "Change it and try to recompile the builder :)"
                ,
                Npm);
        return false;
    }

    if (Builder::Fs::find_executable(std::string(Npx)) == std::filesystem::path {}){
        Builder::log_fmt(Builder::LogLevel_E::ERROR, 
                "Could not locate npx: '{}'\n"
                "Make sure it is either present in PATH env-var or is path to an executable.\n"
                "Defined in builder_src/config.hpp as 'Npx'\n\n"
                "Change it and try to recompile the builder :)"
                ,
                Npx);
        return false;
    }

    return true;
}

bool build_web() {
    Builder::log_fmt(Builder::LogLevel_E::INFO, "Building web...\nVerifying dependencies...");
    if (!verify_web_build_dependencies()) {
        return false;
    }

    std::vector<std::filesystem::path> _cpp_files{};

    if (!Builder::Fs::collect_files_with_extension(Project_Mpp_Lib_Directory, ".cpp", true, _cpp_files, false)) {
        return false;
    }

    if (!Builder::Fs::collect_files_with_extension(Project_Src_Directory, ".cpp", true, _cpp_files, false)) {
        return false;
    }

    if (!Builder::Fs::collect_files_with_extension(Project_Web_Platform_Directory, ".cpp", true, _cpp_files, false)) {
        return false;
    }

    std::vector<std::string> _cmd_args{};
    const auto _temp_wasm_filepath = Project_Build_Directory / "mpp_temp.wasm";
    const auto _wasm_filepath      = Project_Install_Directory / "mpp_web.wasm";
    { // Building wasm
        _cmd_args.assign({
            "-std=c++23", "--target=wasm32-unknown-unknown",
                "-O2",  "-ffreestanding", "-fno-builtin", "-nostdlib",
                "-Wl,--no-entry", "-Wl,--export-dynamic",
                "-DPLATFORM_WEB",
                "-o", _temp_wasm_filepath.string()
#ifdef USE_NIKHIL_CONFIG
                    , "./multi3.o"
#endif
        });
        for (const auto &cpp_file : _cpp_files) {
            _cmd_args.emplace_back(cpp_file.string());
        }

        if (!Builder::Nob::nob_cmd_run_sync_vec(std::string(Clang_Plus_Plus_Compiler), _cmd_args)) {
            return false;
        }
        _cmd_args.clear();
    }

    { // Installing esbuild
        _cmd_args.assign({
                "install", "esbuild", "--prefix", Project_Build_Directory.string(),
                });
        if (!Builder::Nob::nob_cmd_run_sync_vec(std::string(Npm), _cmd_args)) {
            return false;
        }
        _cmd_args.clear();
    }

    const auto _temp_esm_filepath = Project_Build_Directory / "mpp_temp_esm.js";
    const auto _esm_filepath      = Project_Install_Directory / "mpp_web.js";
    { // Creating it
        _cmd_args.assign({
                "esbuild", Project_Web_Platform_Ts_Entry_File, std::string("--outfile=") + _temp_esm_filepath.string()
                });

        if (!Builder::Nob::nob_cmd_run_sync_vec(std::string(Npx), _cmd_args)) {
            return false;
        }
        _cmd_args.clear();
    }

    // Make sure it happens at the end.
    if (!nob_copy_file(_temp_wasm_filepath.c_str(), _wasm_filepath.c_str())) {
        return false;
    }
    if (!nob_copy_file(_temp_esm_filepath.c_str(), _esm_filepath.c_str())) {
        return false;
    }

    return true;
}

bool verify_native_build_dependencies() {
    if (Builder::Fs::find_executable(std::string(Native_Compiler)) == std::filesystem::path {}){
        Builder::log_fmt(Builder::LogLevel_E::ERROR, 
                "Could not locate native compiler: '{}'\n"
                "Make sure it is either present in PATH env-var or is path to an executable.\n"
                "Defined in builder_src/config.hpp as 'Native_Compiler'\n\n"
                "Change it and try to recompile the builder :)"
                ,
                Native_Compiler);
        return false;
    }

    if (Builder::Fs::find_executable(std::string(Llvm_Ar_Linker)) == std::filesystem::path {}){
        Builder::log_fmt(Builder::LogLevel_E::ERROR, 
                "Could not locate llvm-ar linker: '{}'\n"
                "Make sure it is either present in PATH env-var or is path to an executable.\n"
                "Defined in builder_src/config.hpp as 'Llvm_Ar_Linker'\n\n"
                "Change it and try to recompile the builder :)"
                ,
                Llvm_Ar_Linker);
        return false;
    }

    const auto _raylib_header_file = Project_Raylib_Include_Directory / "raylib.h";
    if (!std::filesystem::exists(_raylib_header_file) || !std::filesystem::is_regular_file(_raylib_header_file)) {
        Builder::log_fmt(Builder::LogLevel_E::ERROR, 
                "Could not locate raylib header file: '{}'\n"
                "Make sure it is present and is a file.\n"
                "Parent directory defined in builder_src/config.hpp as 'Project_Raylib_Include_Directory'\n\n"
                "Change it and try to recompile the builder :)"
                ,
                _raylib_header_file.string());
        return false;
    }

    return true;
}

bool build_native() {
    Builder::log_fmt(Builder::LogLevel_E::INFO, "Building native...\nVerifying dependencies...");
    if (!verify_native_build_dependencies()) {
        return false;
    }

    std::vector<std::filesystem::path> _cpp_files{};

    if (!Builder::Fs::collect_files_with_extension(Project_Mpp_Lib_Directory, ".cpp", true, _cpp_files, false)) {
        return false;
    }

    if (!Builder::Fs::collect_files_with_extension(Project_Src_Directory, ".cpp", true, _cpp_files, false)) {
        return false;
    }

    if (!Builder::Fs::collect_files_with_extension(Project_Native_Platform_Directory, ".cpp", true, _cpp_files, false)) {
        return false;
    }

    if (!nob_clean_dir_build(Project_Build_Directory)) {
        return false;
    }

    std::vector<std::string> _cmd_args{};

    std::vector<std::filesystem::path> _obj_files{};
    for (const auto &cpp_file : _cpp_files) {
        _obj_files.emplace_back(Project_Build_Directory / std::format("native_{}.o", _obj_files.size()));

        _cmd_args.assign({
                "-std=c++23", "-c", cpp_file.string(), "-o", _obj_files.back().string(), (std::string("-I") + Project_Raylib_Include_Directory.string())
                });
        if (!Builder::Nob::nob_cmd_run_sync_vec(Native_Compiler.data(), _cmd_args)) {
            return false;
        }
        _cmd_args.clear();
    }

    const auto _library_temp_filepath = Project_Build_Directory / "temp_mpp_native.a";
    const auto _library_filepath = Project_Install_Directory / "mpp_native.a";
    {
        _cmd_args.emplace_back("-rcs");
        _cmd_args.emplace_back(_library_temp_filepath.string());
        for (const auto &obj_file : _obj_files) {
            _cmd_args.emplace_back(obj_file.string());
        }

        if (!Builder::Nob::nob_cmd_run_sync_vec(Llvm_Ar_Linker.data(), _cmd_args)) {
            return false;
        }
    }

    if (!nob_copy_file(_library_temp_filepath.c_str(), _library_filepath.c_str())) {
        return false;
    }

    constexpr std::string_view _header_file_content =
"#ifndef _MPP_NATIVE_GENERATED_H_\n"
"#define _MPP_NATIVE_GENERATED_H_\n"
"#include \"stdint.h\"\n"
"\n"
"#if defined(__cplusplus)\n"
"extern \"C\" {\n"
"#endif\n"
"\n"
"// Logs error upon failure\n"
"// @returns 0 on success\n"
"int32_t project_init();\n"
"\n"
"// @returns true if wants to continue, otherwise terminate execution loop\n"
"uint8_t project_frame_callback();\n"
"\n"
"void    project_close();\n"
"\n"
"#if defined(__cplusplus)\n"
"} // extern \"C\" {\n"
"#endif\n"
"\n"
"/**\n"
" * Simple ussage:\n"
" * // Header includes...\n"
" *\n"
" * ```\n"
" * int main() {\n"
" *     if (project_init() != 0) {\n"
" *         // Handle error\n"
" *         return 1;\n"
" *     }\n"
" *\n"
" *     while (project_frame_callback());\n"
" *\n"
" *     project_close();\n"
" *     return 0;\n"
" * }\n"
" * ```\n"
" */\n"
"\n"
"#endif // _MPP_NATIVE_GENERATED_H_\n"
;

    const auto _header_filepath = (Project_Install_Directory / "mpp_native.h");
    if (!nob_write_entire_file(_header_filepath.c_str(), _header_file_content.data(), _header_file_content.length())) {
        return false;
    } 

    Builder::log_fmt(Builder::LogLevel_E::INFO,
            "Successfully generated (2) files:\n"
            "  Header : {}\n"
            "  Library: {}\n\n"
            "Note: After this to be able to run program you will need raylib's library file and required files to make raylib work for e.g. X11 on posix\n\n"
            "Enjoy!",
            _header_filepath.string(),
            _library_filepath.string()
            );

    return true;
}

int main() {
    Builder::Nob::nob_init();

    if (!nob_clean_dir_build(Project_Install_Directory)) {
        return 1;
    }

    if (!nob_clean_dir_build(Project_Build_Directory)) {
        return 1;
    }

    if (!build_native()) {
        return 1;
    }

    if (!build_web()) {
        return 1;
    }

    return 0;
}
