#pragma once
/*
 * Created by Brett on 29/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_CROW_PCH_H
#define CROWSITE_CROW_PCH_H

#include <crowsite/config.h>
#include "crow/query_string.h"
#include "crow/http_parser_merged.h"
#include "crow/ci_map.h"
#include "crow/TinySHA1.hpp"
#include "crow/settings.h"
#include "crow/socket_adaptors.h"
#include "crow/json.h"
#include "crow/mustache.h"
#include "crow/logging.h"
#include "crow/task_timer.h"
#include "crow/utility.h"
#include "crow/common.h"
#include "crow/http_request.h"
#include "crow/websocket.h"
#include "crow/parser.h"
#include "crow/http_response.h"
#include "crow/multipart.h"
#include "crow/routing.h"
#include "crow/middleware.h"
#include "crow/middleware_context.h"
#include "crow/compression.h"
#include "crow/http_connection.h"
#include "crow/http_server.h"
#include "crow/app.h"
#include "crow/middlewares/session.h"
#include "crow/middlewares/cookie_parser.h"

using Session = crow::SessionMiddleware<crow::FileStore>;
using CrowApp = crow::App<crow::CookieParser, Session>;


#endif //CROWSITE_CROW_PCH_H
