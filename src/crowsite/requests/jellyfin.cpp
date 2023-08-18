//
// Created by brett on 8/9/23.
//
#include <crowsite/requests/jellyfin.h>
#include <crowsite/requests/curl.h>
#include <crow/json.h>
#include <blt/std/hashmap.h>
#include <blt/std/logging.h>

namespace cs::jellyfin
{
    
    struct
    {
        std::string token;
        HASHMAP<std::string, client_data> user_ids;
    } GLOBALS;
    
    void setToken(std::string_view token)
    {
        GLOBALS.token = token;
    }
    
    void processUserData()
    {
        auto usr_data = getUserData();
        
        auto json = crow::json::load(usr_data);
        
        for (const auto& users : json)
        {
            const auto& policy = users["Policy"];
            
            client_data data;
            data.isAdmin = policy["IsAdministrator"].b();
            auto& user = data.user;
            user.name = users["Name"].s();
            user.serverId = users["ServerId"].s();
            user.Id = users["Id"].s();
            user.hasPassword = users["HasPassword"].b();
            user.hasConfiguredPassword = users["HasConfiguredPassword"].b();
            user.hasConfiguredEasyPassword = users["HasConfiguredEasyPassword"].b();
            user.enableAutoLogin = users["EnableAutoLogin"].b();
            user.lastLoginDate = users["LastLoginDate"].s();
            user.lastActivityDate = users["LastActivityDate"].s();
            
            //BLT_TRACE("Processing %s = %s", username.operator std::string().c_str(), userid.operator std::string().c_str());
            GLOBALS.user_ids[user.name] = data;
        }
    }
    
    std::string getUserData()
    {
#define url "https://media.tpgc.me/Users"
        
        cs::request request;
        request.setContentHeaderJson();
        request.setAuthHeader(generateAuthHeader());
        request.get(url);
        
        return cs::request::getResponse(url);
    }
    
    std::string generateAuthHeader()
    {
        return "MediaBrowser Client=Crowsite, Device=YourMom, Token=" + GLOBALS.token;
    }
    
    bool hasUser(std::string_view username)
    {
        return GLOBALS.user_ids.find(std::string(username)) != GLOBALS.user_ids.end();
    }
    
    auth_response authenticateUser(std::string_view username, std::string_view password)
    {
        if (!hasUser(username))
            processUserData();
        if (!hasUser(username))
        {
            BLT_ERROR("User not found!");
            return auth_response::USERNAME;
        }
        auto l_url = "https://media.tpgc.me/Users/AuthenticateByName";
        
        cs::request post;
        post.setContentHeaderJson();
        post.setAuthHeader(generateAuthHeader());
        crow::json::wvalue json;
        json["Username"] = std::string(username);
        json["Pw"] = std::string(password);
        post.post(l_url, json.dump());
        
        auto response = cs::request::getResponseAndClear(l_url);
        
        if (post.status() == 200)
            return auth_response::AUTHORIZED;
        
        return auth_response::ERROR;
    }
    
    const client_data& jellyfin::getUserData(const std::string& username)
    {
        return GLOBALS.user_ids[username];
    }
    
}