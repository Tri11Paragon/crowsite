#pragma once
/*
 * Created by Brett on 29/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_CROW_CONVERSION_H
#define CROWSITE_CROW_CONVERSION_H

#include "crow/http_response.h"
#include <crowsite/util/crow_fix.h>

namespace cs
{
    
    crow::response toResponse(cs::response_info res);
    
    inline crow::response redirect(const std::string& loc, int code = 303)
    {
        crow::response res(code);
        res.set_header("Location", loc);
        return res;
    }
    
}


#endif //CROWSITE_CROW_CONVERSION_H
