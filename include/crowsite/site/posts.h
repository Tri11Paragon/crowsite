#pragma once
/*
 * Created by Brett on 23/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_POSTS_H
#define CROWSITE_POSTS_H

#include <crowsite/site/cache.h>
#include <crow/http_request.h>
#include <crow/http_response.h>

namespace cs
{
    struct request_info {
        CacheEngine& engine;
        const crow::request& req;
        std::string clientID;
        std::string tokenID;
        std::string path;
    };

    crow::response handleProjectPage(const request_info& req);

}

#endif //CROWSITE_POSTS_H
