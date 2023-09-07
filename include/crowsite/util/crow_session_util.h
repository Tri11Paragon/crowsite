#pragma once
/*
 * Created by Brett on 29/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_CROW_SESSION_UTIL_H
#define CROWSITE_CROW_SESSION_UTIL_H

#include "crowsite/site/auth.h"
#include "crow_typedef.h"
#include "crowsite/site/cache.h"

namespace cs
{
    constexpr auto session_age = 24 * 60 * 60;
    constexpr auto cookie_age = 180 * 24 * 60 * 60;
    
    struct site_params
    {
        CrowApp& app;
        cs::CacheEngine& engine;
        const crow::request& req;
        const std::string& name;
    };
    
    /**
     * Note this function destroys the user's session and any login related cookies!
     */
    void destroyUserSession(CrowApp& app, const crow::request& req);
    
    bool checkAndUpdateUserSession(CrowApp& app, const crow::request& req);
    
    bool isUserLoggedIn(CrowApp& app, const crow::request& req);
    
    bool isUserAdmin(CrowApp& app, const crow::request& req);
    
    void generateRuntimeContext(const site_params& params, cs::context& context);
}

#define CS_SESSION cs::checkAndUpdateUserSession(app, req); \
                    auto& session = app.get_context<Session>(req); \
                    auto s_clientID = session.get("clientID", ""); \
                    auto s_clientToken = session.get("clientToken", ""); \

#endif //CROWSITE_CROW_SESSION_UTIL_H
