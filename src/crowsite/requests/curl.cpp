//
// Created by brett on 8/9/23.
//
#include <crowsite/requests/curl.h>
#include <blt/std/logging.h>
#include <blt/std/hashmap.h>

namespace cs {
    
    HASHMAP<std::string, std::string> responses;
    
    void writeData(char *ptr, size_t size, size_t nmemb, void *userdata){
        BLT_INFO("Data %p, %u %u", ptr, size, nmemb);
        const char* name = (const char*) userdata;
        std::string site{name};
        std::string response;
        response.reserve(nmemb);
        for (size_t i = 0; i < nmemb; i++)
            response += ptr[i];
        BLT_TRACE("%s", response.c_str());
        responses[site] = response;
    }
    
    void requests::init()
    {
        auto code = curl_global_init(CURL_GLOBAL_ALL);
        if (code)
        {
            BLT_ERROR("Unable to call CURL init!");
            std::exit(code);
        }
    }
    
    void requests::cleanup()
    {
        curl_global_cleanup();
    }
    
    request::request()
    {
        handler = curl_easy_init();
    }
    
    request::~request()
    {
        curl_slist_free_all(headers);
        curl_easy_cleanup(handler);
    }
    
    void request::setAuthHeader(const std::string& header)
    {
        curl_slist_free_all(headers);
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: " + header).c_str());
        
        curl_easy_setopt(handler, CURLOPT_HTTPHEADER, headers);
    }
    
    void request::get(const std::string& domain)
    {
        curl_easy_setopt(handler, CURLOPT_URL, domain.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEDATA, domain.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, writeData);
        
        // TODO Error decode
        auto err = curl_easy_perform(handler);
        if (err){
            BLT_ERROR("CURL failed to send request '%s'. Error '%s'", domain.c_str(), curl_easy_strerror(err));
        }
    }
    
    void request::post(const std::string& domain)
    {
    
    }
}