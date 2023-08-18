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
            bool hasPassword;
            bool hasConfiguredPassword;
            bool hasConfiguredEasyPassword;
            bool enableAutoLogin;
            std::string lastLoginDate;
            std::string lastActivityDate;
        } user;
        bool isAdmin{};
    };
    
    void setToken(std::string_view token);
    
    void processUserData();
    const client_data& getUserData(const std::string& username);
    
    std::string generateAuthHeader();
    std::string getUserData();
    
    bool hasUser(std::string_view username);
    auth_response authenticateUser(std::string_view username, std::string_view password);
    
}

#endif //CROWSITE_JELLYFIN_H
