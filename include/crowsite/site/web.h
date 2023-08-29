//
// Created by brett on 6/20/23.
//

#ifndef CROWSITE_WEB_H
#define CROWSITE_WEB_H

#include <memory>
#include <string>
#include <crowsite/config.h>
#include <crowsite/util/crow_typedef.h>
#include <utility>

namespace cs {
    
    class HTMLPage {
        private:
            std::string m_SiteData;
            explicit HTMLPage(std::string siteData);
        public:
            static std::unique_ptr<HTMLPage> load(const std::string& path);
            
            inline std::string& getRawSite() {
                return m_SiteData;
            }
    };
    
}

#endif //CROWSITE_WEB_H
