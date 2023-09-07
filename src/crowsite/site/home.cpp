/*
 * Created by Brett on 27/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/site/home.h>
#include <crowsite/util/crow_conversion.h>
#include <blt/std/logging.h>

namespace cs
{
    
    crow::response handle_root_page(const site_params& params)
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
    
    crow::response handle_auth_page(const site_params& params)
    {
        if (isUserAdmin(params.app, params.req))
            return redirect("/login.html");
        
        
        return handle_root_page(params);
    }
    
    crow::response handle_login_request(const crow::request& req, CrowApp& app)
    {
        cs::parser::Post pp(req.body);
        auto& session = app.get_context<Session>(req);
        
        std::string user_agent;
        
        for (const auto& h : req.headers)
            if (h.first == "User-Agent")
            {
                user_agent = h.second;
                break;
            }
        
        // either cs::redirect to clear the form if failed or pass user to index
        if (cs::checkUserAuthorization(pp))
        {
            cs::cookie_data data = cs::createUserAuthTokens(pp, user_agent);
            if (!cs::storeUserData(pp["username"], user_agent, data))
            {
                BLT_ERROR("Failed to update user data");
                return cs::redirect("login.html");
            }
            
            session.set("clientID", data.clientID);
            session.set("clientToken", data.clientToken);
            if (pp.hasKey("remember_me") && pp["remember_me"][0] == 'T')
            {
                auto& cookie_context = app.get_context<crow::CookieParser>(req);
                cookie_context.set_cookie("clientID", data.clientID).path("/").max_age(cookie_age);
                cookie_context.set_cookie("clientToken", data.clientToken).path("/").max_age(cookie_age);
            }
            return cs::redirect(pp.hasKey("referer") ? pp["referer"] : "/");
        } else
            return cs::redirect("login.html");
    }
}