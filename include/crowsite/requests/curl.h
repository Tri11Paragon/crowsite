//
// Created by brett on 8/9/23.
//

#ifndef CROWSITE_CURL_H
#define CROWSITE_CURL_H

#include <string>
#include <curl/curl.h>

namespace cs {

    void init();
    void cleanup();
    
    class request {
        private:
            CURL* handler = nullptr;
            struct curl_slist* headers = nullptr;
        public:
            request();
            void setAuthHeader(const std::string& header);
            void get(const std::string& domain);
            void post(const std::string& domain);
            ~easyrequest_get();
    };

}

#endif //CROWSITE_CURL_H
