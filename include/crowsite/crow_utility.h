//
// Created by brett on 21/08/23.
//

#ifndef CROWSITE_CROW_UTILITY_H
#define CROWSITE_CROW_UTILITY_H

#include <crow/http_response.h>

namespace cs {
    inline crow::response redirect(const std::string& loc = "/", int code = 303)
    {
        crow::response res(code);
        res.set_header("Location", loc);
        return res;
    }
}

#endif //CROWSITE_CROW_UTILITY_H
