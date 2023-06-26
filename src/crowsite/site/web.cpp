//
// Created by brett on 6/20/23.
//
#include <crowsite/site/web.h>
#include <fstream>
#include <ios>
#include <blt/std/logging.h>
#include <boost/algorithm/string/replace.hpp>

#include <utility>
#include <sstream>

namespace cs {
    
    std::unique_ptr<HTMLPage> HTMLPage::load(const std::string& path) {
        std::string htmlSource;
        std::ifstream htmlFile;
        if (!htmlFile.good())
            BLT_ERROR("Input stream not good!\n");
        // ensure we can throw exceptions:
        htmlFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            // open file
            htmlFile.open(path);
            std::stringstream htmlAsStream;
            // read file's buffer contents into streams
            htmlAsStream << htmlFile.rdbuf();
            // close file handlers
            htmlFile.close();
            // convert stream into std::string
            htmlSource = htmlAsStream.str();
        } catch (std::ifstream::failure& e) {
            BLT_ERROR("Unable to read file '%s'!\n", path.c_str());
            BLT_ERROR("Exception: %s", e.what());
            throw std::runtime_error("Failed to read file!\n");
        }
        return std::make_unique<HTMLPage>(HTMLPage(htmlSource));
    }
    
    HTMLPage::HTMLPage(std::string siteData): m_SiteData(std::move(siteData)) {}
    
    class SyntaxException : public std::runtime_error {
        public:
            explicit SyntaxException(): std::runtime_error("Extended-mustache syntax error! Static context keys should not contain $") {}
    };
    
    std::string HTMLPage::render(StaticContext& context) {
        std::string processedSiteData = m_SiteData;
        
        for (auto& v : context){
            if (v.first.starts_with('$'))
                throw SyntaxException();
            boost::replace_all(processedSiteData, "{{$" + v.first + "}}", v.second);
        }
        
        return processedSiteData;
    }
    
    void HTMLPage::resolveResources() {
    
    }
}
