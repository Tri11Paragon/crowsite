//
// Created by brett on 6/20/23.
//

#ifndef CROWSITE_CONFIG_H
#define CROWSITE_CONFIG_H

#include <unordered_map>

template<typename K, typename V, typename HASH=std::hash<K>>
using hashmap = std::unordered_map<K, V, HASH>;

#define CROW_STATIC_DIRECTORY "/mnt/games/Projects/web/crowsite_test/static/"
#define SITE_FILES_PATH "/mnt/games/Projects/web/crowsite_test"
#define CROW_STATIC_ENDPOINT "/static/<path>"

#define MILES_SITE
#undef MILES_SITE


#endif //CROWSITE_CONFIG_H
