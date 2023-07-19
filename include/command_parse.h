#pragma once

#include <iostream>
#include <filesystem>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

namespace command_line{

namespace fs = std::filesystem;

struct Args {
    boost::optional<uint32_t> period;
    fs::path config_file;
    fs::path source_dir;
    bool random_spawn = false;
    boost::optional<fs::path> state_file;
    boost::optional<int64_t> save_state_period;
};

std::optional<Args> ParseCommandLine(int argc, const char* const argv[]);

}