#define NOB_IMPLEMENTATION
#include "./utility.hpp"
#include "./config.hpp"

bool nob_clean_dir_build(const std::filesystem::path &path) {
    if (!Builder::Fs::remove(path, true)) {
        return false;
    }
    return nob_mkdir_if_not_exists(path.c_str());
}

bool build_native_library() {
    std::vector<std::filesystem::path> _cpp_files{};

    if (!Builder::Fs::collect_files_with_extension(Project_Mpp_Lib_Directory, ".cpp", true, _cpp_files, false)) {
        return false;
    }

    if (!Builder::Fs::collect_files_with_extension(Project_Src_Directory, ".cpp", true, _cpp_files, false)) {
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
                "-std=c++23", "-c", cpp_file.string(), "-o", _obj_files.back().string()
                });
        if (!Builder::Nob::nob_cmd_run_sync_vec(Builder_Native_Compiler.data(), _cmd_args)) {
            return false;
        }
        _cmd_args.clear();
    }

    if (!nob_clean_dir_build(Project_Install_Directory)) {
        return false;
    }

    {
        _cmd_args.emplace_back("-rcs");
        _cmd_args.emplace_back((Project_Install_Directory / "mpp_native.a").string());
        for (const auto &obj_file : _obj_files) {
            _cmd_args.emplace_back(obj_file.string());
        }

        if (!Builder::Nob::nob_cmd_run_sync_vec(Llvm_Ar_Linker.data(), _cmd_args)) {
            return false;
        }
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

    if (!nob_write_entire_file((Project_Install_Directory / "mpp_native.h").c_str(), _header_file_content.data(), _header_file_content.length())) {
        return false;
    } 

    Builder::log_fmt(Builder::LogLevel_E::INFO,
            "Successfully generated native library files (.a and .h)\n"
            "Note: Link with libraylib.a that suits your platform (Not mandatory to use one from ./vendor/raylib/lib/\n"
            "Note: Link with necessary library files for e.g. -lX11\n"
            "\n"
            "Enjoy!"
            );

    return true;
}

int main() {
    Builder::Nob::nob_init();

    if (!build_native_library()) {
        return 1;
    }

    return 0;
}
