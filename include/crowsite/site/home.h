#pragma once
/*
 * Created by Brett on 27/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_HOME_H
#define CROWSITE_HOME_H

#include <crowsite/util/crow_session_util.h>

namespace cs
{
 
    crow::response handle_login_request(const crow::request& req, CrowApp& app);
    
    crow::response handle_root_page(const site_params& params);
    
    crow::response handle_auth_page(const site_params& params);
    
}

#endif //CROWSITE_HOME_H
