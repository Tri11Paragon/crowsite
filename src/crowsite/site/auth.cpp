//
// Created by brett on 16/08/23.
//
#include <crowsite/site/auth.h>
#include <crowsite/requests/jellyfin.h>
#include "blt/std/logging.h"
#include "blt/std/uuid.h"

using namespace blt;

namespace cs {
    
    bool handleLoginPost(parser::Post& postData, cookie_data& cookieOut)
    {
        // javascript should make sure we don't send post requests without information
        // this way it can be interactive
        if (!postData.hasKey("username") || !postData.hasKey("password"))
            return false;
        auto auth = jellyfin::authenticateUser(postData["username"], postData["password"]);
        
        cookieOut.clientID = uuid::toString(uuid::genV5("ClientID?"));
        cookieOut.clientToken = uuid::toString(uuid::genV4());
        
        return auth == jellyfin::auth_response::AUTHORIZED;
    }
}