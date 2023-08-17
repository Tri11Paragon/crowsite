//
// Created by brett on 16/08/23.
//
#include <crowsite/site/auth.h>
#include <crowsite/requests/jellyfin.h>
#include "blt/std/logging.h"

namespace cs {
    
    bool handleLoginPost(parser::Post& postData)
    {
        // javascript should make sure we don't send post requests without information
        // this way it can be interactive
        if (!postData.hasKey("username") || !postData.hasKey("password"))
            return false;
        auto auth = jellyfin::authenticateUser(postData["username"], postData["password"]);
        
        return auth == jellyfin::auth_response::AUTHORIZED;
    }
}