#pragma once
/*
 * Created by Brett on 28/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_CROW_TYPEDEF_H
#define CROWSITE_CROW_TYPEDEF_H

#include <string>

namespace cs
{
    typedef HASHMAP<std::string, std::string> header_map;
    typedef HASHMAP<std::string, std::string> query_string;
    typedef HASHMAP<std::string, std::string> context;
}

#endif //CROWSITE_CROW_TYPEDEF_H
