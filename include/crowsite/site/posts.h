#pragma once
/*
 * Created by Brett on 23/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_POSTS_H
#define CROWSITE_POSTS_H

#include <crowsite/site/cache.h>
#include <crowsite/util/crow_fix.h>

namespace cs
{
    void posts_init();
    void posts_cleanup();
    
    response_info handleProjectPage(const request_info& req);
}

#endif //CROWSITE_POSTS_H
