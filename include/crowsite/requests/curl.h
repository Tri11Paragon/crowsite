//
// Created by brett on 8/9/23.
//

#ifndef CROWSITE_CURL_H
#define CROWSITE_CURL_H

#include <string>
#include <curl/curl.h>

namespace cs
{
    
    namespace requests
    {
        void init();
        
        void cleanup();
    }
    
    class request
    {
        private:
            CURL* handler = nullptr;
            struct curl_slist* headers = nullptr;
        public:
            request();
            
            static const std::string& getResponse(const std::string& domain);
            static std::string getResponseAndClear(const std::string& domain);
            static void clearResponse(const std::string& domain);
            
            void setContentHeaderJson();
            void setContentHeaderXForm();
            void setContentHeader(const std::string& header);
            void setAuthHeader(const std::string& header);
            
            void get(const std::string& domain, const std::string& data = "");
            
            void post(const std::string& domain, const std::string& data = "");
            
            long status();
            
            ~request();
    };
    
}

#endif //CROWSITE_CURL_H
