#pragma once

#include <boost/json.hpp>

namespace logger{

namespace json = boost::json;

void InitBoostLogs();

void Info(const std::string& message, const json::value& custom_data);

void Error(const std::string& message, const json::value& custom_data);

void Fatal(const std::string& message, const json::value& custom_data);

}