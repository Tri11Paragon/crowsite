/*
 * Created by Brett on 01/09/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/site/routing.h>
#include <blt/std/logging.h>
#include "crowsite/util/crow_session_util.h"
#include "crowsite/utility.h"
#include "crowsite/util/crow_conversion.h"
#include "crowsite/site/home.h"

namespace cs
{
    inline void createLoginRoutes(CrowApp& app, CacheEngine& engine)
    {
        CROW_ROUTE(app, "/login.html")(
                [&app, &engine](const crow::request& req) -> crow::response {
                    if (cs::isUserLoggedIn(app, req))
                        return cs::redirect("/");
                    return cs::handle_root_page({app, engine, req, "login.html"});
                }
        );
        
        CROW_ROUTE(app, "/logout.html")(
                [&app](const crow::request& req) -> crow::response {
                    cs::destroyUserSession(app, req);
                    return cs::redirect("/");
                }
        );
        
        CROW_ROUTE(app, "/res/login").methods(crow::HTTPMethod::POST)(
                [&app](const crow::request& req) {
                    return cs::handle_login_request(req, app);
                }
        );
    }
    
    void establishHomeRoutes(CrowApp& app, CacheEngine& engine)
    {
        CROW_ROUTE(app, "/favicon.ico")(
                [](crow::response& local_fav_res) {
                    local_fav_res.compressed = false;
                    local_fav_res.set_static_file_info_unsafe(cs::fs::createStaticFilePath("images/favicon.ico"));
                    local_fav_res.set_header("content-type", "image/x-icon");
                    local_fav_res.end();
                }
        );
        
        createLoginRoutes(app, engine);
        
        CROW_ROUTE(app, "/<string>")(
                [&app, &engine](const crow::request& req, const std::string& name) -> crow::response {
                    return cs::handle_root_page({app, engine, req, name});
                }
        );
        
        CROW_ROUTE(app, "/")(
                [&engine, &app](const crow::request& req) {
                    return cs::handle_root_page({app, engine, req, "index.html"});
                }
        );
    }
    
    void establishProjectRoutes(CrowApp& app, CacheEngine& engine)
    {
    
    }
}