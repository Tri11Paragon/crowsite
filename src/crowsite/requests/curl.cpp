//
// Created by brett on 8/9/23.
//
#include <crowsite/requests/curl.h>
#include <blt/std/logging.h>
#include <blt/std/hashmap.h>
#include <blt/std/memory.h>
#include <cstring>

namespace cs
{
    
    HASHMAP<std::string, std::string> responses;
    
    size_t writeData(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        auto* name = (const char*) userdata;
        std::string site{name};
        
        blt::scoped_buffer<char> response{size * nmemb + 1};
        memcpy(response.ptr(), ptr, size * nmemb);
        response[size * nmemb] = '\0';
        
        if (responses.find(site) != responses.end()){
            std::string res{response.ptr()};
            responses[site].append(res);
        } else
            responses[site] = std::string(response.ptr());
        
        return size * nmemb;
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
        headers = curl_slist_append(headers, ("Authorization: " + header).c_str());
        
        curl_easy_setopt(handler, CURLOPT_HTTPHEADER, headers);
    }
    
    void request::get(const std::string& domain, const std::string& data)
    {
        auto full = domain + data;
        curl_easy_setopt(handler, CURLOPT_URL, full.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEDATA, domain.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, writeData);
        
        auto err = curl_easy_perform(handler);
        if (err != CURLE_OK)
        {
            BLT_ERROR("CURL failed to send GET request '%s'. Error '%s'", domain.c_str(), curl_easy_strerror(err));
        }
    }
    
    void request::post(const std::string& domain, const std::string& data)
    {
        curl_easy_setopt(handler, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(handler, CURLOPT_URL, domain.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEDATA, domain.c_str());
        curl_easy_setopt(handler, CURLOPT_WRITEFUNCTION, writeData);
        
        auto err = curl_easy_perform(handler);
        if (err != CURLE_OK)
        {
            BLT_ERROR("CURL failed to send POST request '%s'. Error '%s'", domain.c_str(), curl_easy_strerror(err));
        }
    }
    
    const std::string& request::getResponse(const std::string& domain)
    {
        return responses[domain];
    }
    
    void request::clearResponse(const std::string& domain)
    {
        responses.erase(domain);
    }
    
    std::string request::getResponseAndClear(const std::string& domain)
    {
        std::string res = getResponse(domain);
        clearResponse(domain);
        return res;
    }
    
    void request::setContentHeaderJson()
    {
        setContentHeader("application/json");
    }
    
    void request::setContentHeaderXForm()
    {
        setContentHeader("application/x-www-form-urlencoded");
    }
    
    void request::setContentHeader(const std::string& header)
    {
        headers = curl_slist_append(headers, ("Content-Type: " + header).c_str());
    }
    
    long request::status()
    {
        long code = 0;
        curl_easy_getinfo(handler, CURLINFO_RESPONSE_CODE, &code);
        return code;
    }
}