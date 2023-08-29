#pragma once
/*
 * Created by Brett on 29/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_CROW_LOG_H
#define CROWSITE_CROW_LOG_H

#include <string>
#include <crow/logging.h>

class BLT_CrowLogger : public crow::ILogHandler
{
    public:
        void log(std::string message, crow::LogLevel crow_level) final;
};

#endif //CROWSITE_CROW_LOG_H
