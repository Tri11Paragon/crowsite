/*
 * Created by Brett on 29/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/util/crow_session_util.h>
#include <crowsite/crow_pch.h>

namespace cs
{
    
    void destroyUserSession(CrowApp& app, const crow::request& req)
    {
        auto& session = app.get_context<Session>(req);
        auto& cookie_context = app.get_context<crow::CookieParser>(req);
        
        session.set("clientID", "");
        session.set("clientToken", "");
        cookie_context.set_cookie("clientID", "");
        cookie_context.set_cookie("clientToken", "");
    }
    
    bool checkAndUpdateUserSession(CrowApp& app, const crow::request& req)
    {
        auto& session = app.get_context<Session>(req);
        auto& cookie_context = app.get_context<crow::CookieParser>(req);
        
        auto s_clientID = session.get("clientID", "");
        auto s_clientToken = session.get("clientToken", "");
        
        auto c_clientID = cookie_context.get_cookie("clientID");
        auto c_clientToken = cookie_context.get_cookie("clientToken");
        
        if ((!c_clientID.empty() && !c_clientToken.empty()) && (s_clientID != c_clientID || s_clientToken != c_clientToken))
        {
            session.set("clientID", c_clientID);
            session.set("clientToken", c_clientToken);
            return true;
        }
        return false;
    }
    
    bool isUserLoggedIn(CrowApp& app, const crow::request& req)
    {
        auto& session = app.get_context<Session>(req);
        auto s_clientID = session.get("clientID", "");
        auto s_clientToken = session.get("clientToken", "");
        return cs::isUserLoggedIn(s_clientID, s_clientToken);
    }
    
    bool isUserAdmin(CrowApp& app, const crow::request& req)
    {
        auto& session = app.get_context<Session>(req);
        auto s_clientID = session.get("clientID", "");
        return cs::isUserAdmin(cs::getUserFromID(s_clientID));
    }
    
    void generateRuntimeContext(const site_params& params, cs::context& context)
    {
        auto& session = params.app.get_context<Session>(params.req);
        auto s_clientID = session.get("clientID", "");
        auto s_clientToken = session.get("clientToken", "");
        if (cs::isUserLoggedIn(s_clientID, s_clientToken))
        {
            auto username = cs::getUserFromID(s_clientID);
            auto perms = cs::getUserPermissions(username);
            auto isAdmin = cs::isUserAdmin(username);
            context["_logged_in"] = "True";
            context["_username"] = username;
            if (isAdmin)
                context["_admin"] = "True";
            if (perms & cs::PERM_READ_FILES)
                context["_read_files"] = "True";
            if (perms & cs::PERM_WRITE_FILES)
                context["_write_files"] = "True";
            if (perms & cs::PERM_CREATE_POSTS)
                context["_create_posts"] = "True";
            if (perms & cs::PERM_CREATE_COMMENTS)
                context["_create_comments"] = "True";
            if (perms & cs::PERM_CREATE_SHARES)
                context["_create_shares"] = "True";
            if (perms & cs::PERM_EDIT_POSTS)
                context["_edit_posts"] = "True";
            if (perms & cs::PERM_EDIT_COMMENTS)
                context["_edit_comments"] = "True";
        }
    }
}
