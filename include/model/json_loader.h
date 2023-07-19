#pragma once

#include <filesystem>
#include "game.h"
#include "command_parse.h"

namespace json_loader {

model::Game LoadGame(const command_line::Args& args);

}  // namespace json_loader