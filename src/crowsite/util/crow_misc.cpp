/*
 * Created by Brett on 29/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/util/crow_log.h>
#include <crowsite/util/crow_conversion.h>
#include <blt/std/logging.h>

void BLT_CrowLogger::log(std::string message, crow::LogLevel crow_level)
{
    blt::logging::log_level blt_level = blt::logging::log_level::NONE;
    switch (crow_level)
    {
        case crow::LogLevel::DEBUG:
            blt_level = blt::logging::log_level::DEBUG;
            break;
        case crow::LogLevel::INFO:
            blt_level = blt::logging::log_level::INFO;
            break;
        case crow::LogLevel::WARNING:
            blt_level = blt::logging::log_level::WARN;
            break;
        case crow::LogLevel::ERROR:
            blt_level = blt::logging::log_level::ERROR;
            break;
        case crow::LogLevel::CRITICAL:
            blt_level = blt::logging::log_level::FATAL;
            break;
    }
    BLT_LOG("Crow: %s", blt_level, message.c_str());
}

namespace cs
{
    
    crow::response toResponse(cs::response_info res)
    {
        if (res.ctx.has_value())
        {
            auto v = crow::mustache::compile(res.body);
            crow::mustache::context ctx;
            for (const auto& c : res.ctx.value())
                ctx[c.first] = c.second;
            return v.render(ctx);
        }
        return res.body;
    }
    
}