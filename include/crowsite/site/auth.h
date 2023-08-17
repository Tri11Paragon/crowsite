//
// Created by brett on 16/08/23.
//

#ifndef CROWSITE_AUTH_H
#define CROWSITE_AUTH_H

#include "crowsite/utility.h"
#include <string>

namespace cs {
    
    struct cookie_data {
        std::string clientID;
        std::string clientToken;
    };

    bool handleLoginPost(cs::parser::Post& postData, cookie_data& cookieOut);

}

#endif //CROWSITE_AUTH_H
