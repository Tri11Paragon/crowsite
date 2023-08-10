//
// Created by brett on 8/9/23.
//

#ifndef CROWSITE_CURL_H
#define CROWSITE_CURL_H

#include <string>
#include <curl/curl.h>

namespace cs::requests {

    void init();
    void cleanup();
    
    class easy_get {
        private:
            CURL* handler = nullptr;
            struct curl_slist* headers = nullptr;
        public:
            easy_get();
            void setAuthHeader(const std::string& header);
            void request(const std::string& domain);
            ~easy_get();
    };

}

#endif //CROWSITE_CURL_H
