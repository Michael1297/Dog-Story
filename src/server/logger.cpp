#include "server/logger.h"
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/date_time.hpp>


namespace logs = boost::log;
namespace keywords = boost::log::keywords;
namespace json = boost::json;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

void JsonFormatter(const logs::record_view& record, logs::formatting_ostream& strm) {
    json::object jv;

    jv["timestamp"] = to_iso_extended_string(*record[timestamp]);
    jv["message"] = *record[expr::smessage];

    if (!record[additional_data].empty()) {
        jv["data"] = *record[additional_data];
    }

    strm << json::serialize(jv) << std::endl;
}


namespace logger{

void InitBoostLogs(){
    logs::add_common_attributes();

    logs::add_console_log(
            std::cout,
            keywords::auto_flush = true,
            keywords::format = &JsonFormatter
    );

#ifdef DISABLE_LOGGER
    logs::core::get()->set_filter(
            logs::trivial::severity > logs::trivial::info
    );
#endif
};

void Info(const std::string& message, const json::value& custom_data){
    BOOST_LOG_TRIVIAL(info) << logs::add_value(additional_data, custom_data) << message;
}

void Error(const std::string& message, const json::value& custom_data){
    BOOST_LOG_TRIVIAL(error) << logs::add_value(additional_data, custom_data) << message;
}

void Fatal(const std::string& message, const json::value& custom_data){
    BOOST_LOG_TRIVIAL(fatal) << logs::add_value(additional_data, custom_data) << message;
}
}