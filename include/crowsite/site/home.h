#pragma once
/*
 * Created by Brett on 27/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_HOME_H
#define CROWSITE_HOME_H

#include <crowsite/site/cache.h>
#include <crowsite/util/crow_session_util.h>
#include <crowsite/util/crow_conversion.h>

namespace cs
{
    
    inline crow::response handle_root_page(const site_params& params)
    {
        //auto page = crow::mustache::load("index.html"); //
        //return "<html><head><title>Hello There</title></head><body><h1>Suck it " + name + "</h1></body></html>";
//                BLT_TRACE(req.body);
//                for (const auto& h : req.headers)
//                    BLT_TRACE("Header: %s = %s", h.first.c_str(), h.second.c_str());
//                BLT_TRACE(req.raw_url);
//                BLT_TRACE(req.url);
//                BLT_TRACE(req.remote_ip_address);
//                for (const auto& v : req.url_params.keys())
//                    BLT_TRACE("URL: %s = %s", v.c_str(), req.url_params.get(v));
        if (params.name.ends_with(".html"))
        {
            checkAndUpdateUserSession(params.app, params.req);
            auto& session = params.app.get_context<Session>(params.req);
            auto s_clientID = session.get("clientID", "");
            auto s_clientToken = session.get("clientToken", "");
            auto user_perms = cs::getUserPermissions(cs::getUserFromID(s_clientID));
            
            crow::mustache::context ctx;
            cs::context context;
            
            generateRuntimeContext(params, context);
            
            // pass perms in
            if (user_perms & cs::PERM_ADMIN)
                ctx["_admin"] = true;
            
            if (cs::isUserLoggedIn(s_clientID, s_clientToken))
            {
                ctx["_logged_in"] = true;
            } else
            {
                ctx["_not_logged_in"] = true;
            }
            
            // we don't want to pass all get parameters to the context to prevent leaking information
            auto referer = params.req.url_params.get("referer");
            if (referer)
                ctx["referer"] = referer;
            auto page = crow::mustache::compile(params.engine.fetch(params.name, context));
            return page.render(ctx);
        }
        
        return params.engine.fetch("default.html");
    }
    
    inline crow::response handle_auth_page(const site_params& params)
    {
        if (isUserAdmin(params.app, params.req))
            return redirect("/login.html");
        
        
        return handle_root_page(params);
    }
    
}

#endif //CROWSITE_HOME_H
