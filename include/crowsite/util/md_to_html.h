#pragma once
/*
 * Created by Brett on 27/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_MD_TO_HTML_H
#define CROWSITE_MD_TO_HTML_H

#include <string>

namespace cs
{
    std::string loadMarkdownAsHTML(const std::string& path);
}

#endif //CROWSITE_MD_TO_HTML_H
