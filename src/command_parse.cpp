#include "command_parse.h"

namespace command_line{

using namespace std::literals;

std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc{"All options"s};
    Args args;

    // Allowed options:
    // -h [ --help ]                     produce help message
    // -t [ --tick-period ] milliseconds set tick period
    // -c [ --config-file ] file         set config file path
    // -w [ --www-root ] dir             set static files root
    // --randomize-spawn-points          spawn dogs at random positions
    // --state-file                      set file for save and restore game state
    // --save-state-period               set save game state period

    desc.add_options()
        ("help,h", "Show help")
        ("tick-period,t", po::value(&args.period)->value_name("milliseconds"), "set tick period")
        ("config-file,c", po::value(&args.config_file)->value_name("file"s), "set config file path")
        ("www-root,w", po::value(&args.source_dir)->value_name("dir"s), "set static files root")
        ("randomize-spawn-points", po::value<bool>(&args.random_spawn), "spawn dogs at random positions")
        ("state-file", po::value(&args.state_file)->value_name("file"s), "set file for save and Restore game state")
        ("save-state-period", po::value(&args.save_state_period)->value_name("milliseconds"), "set save game state period");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help"s)) {
        std::cout << desc;
        return std::nullopt;
    }

    if(!vm.contains("config-file"s)){
        throw std::runtime_error("Config file have not been specified"s);
    }

    if(!vm.contains("www-root"s)){
        throw std::runtime_error( "Static content path is not specified"s);
    }

    return args;
}

}