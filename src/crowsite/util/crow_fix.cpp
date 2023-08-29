/*
 * Created by Brett on 27/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/util/crow_fix.h>

namespace cs
{
    
    query_string toQueryString(const std::vector<char*>& key_values)
    {
        query_string query_kv;
        for (const auto& kv : key_values)
        {
            std::string str(kv);
            auto equ_loc = str.find('=');
            query_kv[str.substr(0, equ_loc)] = str.substr(equ_loc+1, str.size());
        }
        return query_kv;
    }
}