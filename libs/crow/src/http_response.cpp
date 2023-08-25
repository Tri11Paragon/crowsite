#include <crow/http_response.h>

namespace crow
{
    
    void response::set_static_file_info_unsafe(std::string path)
    {
        file_info.path = path;
        file_info.statResult = stat(file_info.path.c_str(), &file_info.statbuf);
#ifdef CROW_ENABLE_COMPRESSION
        compressed = false;
#endif
        if (file_info.statResult == 0 && S_ISREG(file_info.statbuf.st_mode))
        {
            std::size_t last_dot = path.find_last_of(".");
            std::string extension = path.substr(last_dot + 1);
            code = 200;
            this->add_header("Content-Length", std::to_string(file_info.statbuf.st_size));
            
            if (!extension.empty())
            {
                this->add_header("Content-Type", get_mime_type(extension));
            }
        } else
        {
            code = 404;
            file_info.path.clear();
        }
    }
    
    void response::set_static_file_info(std::string path)
    {
        utility::sanitize_filename(path);
        set_static_file_info_unsafe(path);
    }
    
    void response::end()
    {
        if (!completed_)
        {
            completed_ = true;
            if (skip_body)
            {
                set_header("Content-Length", std::to_string(body.size()));
                body = "";
                manual_length_header = true;
            }
            if (complete_request_handler_)
            {
                complete_request_handler_();
            }
        }
    }
    
    void response::clear()
    {
        body.clear();
        code = 200;
        headers.clear();
        completed_ = false;
        file_info = static_file_info{};
    }
    
    std::string response::get_mime_type(const std::string& contentType)
    {
        const auto mimeTypeIterator = mime_types.find(contentType);
        if (mimeTypeIterator != mime_types.end())
        {
            return mimeTypeIterator->second;
        } else if (validate_mime_type(contentType))
        {
            return contentType;
        } else
        {
            CROW_LOG_WARNING << "Unable to interpret mime type for content type '" << contentType << "'. Defaulting to text/plain.";
            return "text/plain";
        }
    }
    
    bool response::validate_mime_type(const std::string& candidate) noexcept
    {
        // Here we simply check that the candidate type starts with
        // a valid parent type, and has at least one character afterwards.
        std::array<std::string, 10> valid_parent_types = {
                "application/", "audio/", "font/", "example/",
                "image/", "message/", "model/", "multipart/",
                "text/", "video/"};
        for (const std::string& parent : valid_parent_types)
        {
            // ensure the candidate is *longer* than the parent,
            // to avoid unnecessary string comparison and to
            // reject zero-length subtypes.
            if (candidate.size() <= parent.size())
            {
                continue;
            }
            // strncmp is used rather than substr to avoid allocation,
            // but a string_view approach would be better if Crow
            // migrates to C++17.
            if (strncmp(parent.c_str(), candidate.c_str(), parent.size()) == 0)
            {
                return true;
            }
        }
        return false;
    }
    
}
