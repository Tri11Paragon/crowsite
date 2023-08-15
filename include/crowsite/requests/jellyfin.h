//
// Created by brett on 8/9/23.
//

#ifndef CROWSITE_JELLYFIN_H
#define CROWSITE_JELLYFIN_H

#include <string_view>

namespace cs::jellyfin
{
    
    void setToken(std::string_view token);
    
    void processUserData();
    
    std::string generateAuthHeader();
    std::string getUserData();
    
    bool hasUser(std::string_view username);
    
}

#endif //CROWSITE_JELLYFIN_H
