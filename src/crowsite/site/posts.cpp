/*
 * Created by Brett on 23/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/site/posts.h>
#include <crowsite/utility.h>
#include <crowsite/util/md_to_html.h>
#include <blt/std/logging.h>

namespace cs
{
    
    crow::response handleProjectPage(const request_info& req)
    {
        std::string buffer;
        buffer += "<html><head></head><body>";
        auto htmlData = loadMarkdownAsHTML(cs::fs::createDataFilePath("Billionaire Propaganda.md"));
        buffer += htmlData;
        buffer += "</body>";
        
        return buffer;
    }
    
    
    
}