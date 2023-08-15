//
// Created by brett on 8/9/23.
//
#include <crowsite/requests/jellyfin.h>
#include <crowsite/requests/curl.h>
#include <crow/json.h>
#include <blt/std/hashmap.h>
#include <blt/std/logging.h>
#include <string>

struct
{
    std::string token;
    HASHMAP<std::string, std::string> user_ids;
} GLOBALS;

void cs::jellyfin::setToken(std::string_view token)
{
    GLOBALS.token = token;
}

void cs::jellyfin::processUserData()
{
    auto data = getUserData();

    auto json = crow::json::load(data);
    
    for (const auto& user : json){
        auto username = user["Name"].s();
        auto userid = user["Id"].s();
        //BLT_TRACE("Processing %s = %s", username.operator std::string().c_str(), userid.operator std::string().c_str());
        GLOBALS.user_ids[username] = userid;
    }
}

std::string cs::jellyfin::getUserData()
{
#define url "https://media.tpgc.me/Users"
    
    cs::request request;
    request.setAuthHeader(generateAuthHeader());
    request.get(url);
    
    return cs::request::getResponse(url);
}

std::string cs::jellyfin::generateAuthHeader()
{
    return "MediaBrowser Client=Crowsite, Device=YourMom, Token=" + GLOBALS.token;
}

bool cs::jellyfin::hasUser(std::string_view username)
{
    return GLOBALS.user_ids.find(std::string(username)) != GLOBALS.user_ids.end();
}
