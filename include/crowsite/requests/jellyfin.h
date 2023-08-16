//
// Created by brett on 8/9/23.
//

#ifndef CROWSITE_JELLYFIN_H
#define CROWSITE_JELLYFIN_H

#include <string_view>
#include <string>

namespace cs::jellyfin
{
    enum class auth_response {
            AUTHORIZED,
            USERNAME,
            ERROR
    };
    
    struct client_data {
        struct {
            std::string name;
            std::string serverId;
            std::string Id;
            std::string primaryImageTag;
            bool hasPassword;
            bool hasConfiguredPassword;
            bool hasConfiguredEasyPassword;
            bool enableAutoLogin;
            std::string lastLoginDate;
            std::string lastActivityDate;
        } user;
        std::string accessToken;
    };
    
    void setToken(std::string_view token);
    
    void processUserData();
    
    std::string generateAuthHeader();
    std::string getUserData();
    
    bool hasUser(std::string_view username);
    auth_response authenticateUser(std::string_view username, std::string_view password);
    
}

#endif //CROWSITE_JELLYFIN_H
