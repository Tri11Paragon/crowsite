#pragma once
/*
 * Created by Brett on 01/09/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_ROUTING_H
#define CROWSITE_ROUTING_H

#include <crowsite/crow_pch.h>
#include "crowsite/site/cache.h"

namespace cs
{
    void establishHomeRoutes(CrowApp& app, CacheEngine& engine);
    void establishProjectRoutes(CrowApp& app, CacheEngine& engine);
}

#endif //CROWSITE_ROUTING_H
