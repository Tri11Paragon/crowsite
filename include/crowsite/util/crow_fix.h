#pragma once
/*
 * Created by Brett on 27/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_CROW_FIX_H
#define CROWSITE_CROW_FIX_H

/**
 * Basically waste a bunch of time to both a. reduce the compile time of using crow
 * and b. reduce the runtime overhead of crow, by increasing runtime in some respects
 * TODO: explain why this is better later, maybe test as well.
 */

#include <vector>
#include <optional>
#include <blt/std/hashmap.h>
#include "crowsite/site/cache.h"
#include "crowsite/util/crow_typedef.h"

namespace cs
{
    query_string toQueryString(const std::vector<char*>& kv);
    
    struct request_info
    {
        std::string raw_url;     ///< The full URL containing the `?` and URL parameters.
        std::string clientID;
        std::string tokenID;
        std::string path;
        query_string url_params; ///< The parameters associated with the request. (everything after the `?` in the URL)
        CacheEngine& engine;
        std::optional<header_map> headers{};
    };
    
    struct response_info
    {
        std::string body;
        std::optional<context> ctx{};
    };
}

#endif //CROWSITE_CROW_FIX_H
