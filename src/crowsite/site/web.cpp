//
// Created by brett on 6/20/23.
//
#include <crowsite/site/web.h>
#include <fstream>
#include <ios>
#include <blt/std/logging.h>
#include <blt/std/string.h>

#include <utility>
#include <sstream>

namespace cs {
    
    std::unique_ptr<HTMLPage> loadHTMLPage(const std::string& path) {
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
        return std::make_unique<HTMLPage>(htmlSource);
    }
    
    HTMLPage::HTMLPage(std::string siteData): rawSite(std::move(siteData)) {}
    
    class SyntaxException : public std::runtime_error {
        public:
            explicit SyntaxException(): std::runtime_error("Extended-mustache syntax error!") {}
    };
    
    unsigned long noNeg(unsigned long val, unsigned long subtract) {
        if (val - subtract > val)
            return 0;
        return val - subtract;
    }
    
    std::string HTMLPage::render(StaticContext& context) {
        processedSite = rawSite;
        
        auto replacementLocation = processedSite.find(opening);
        while (replacementLocation != std::string::npos) {
            auto searchLocation = replacementLocation+opening.size();
            // for the secondary check we should also look for standard mustache endings
            auto secondaryCheckLocation = processedSite.find("{{", searchLocation);
            // then look for the end starting beyond the beginning of the tag
            auto endingLocation = processedSite.find("}}", searchLocation);
            // rule out non-ending tags and tags which contain other tags (this isn't supported) TODO:
            if (endingLocation == std::string::npos || secondaryCheckLocation < endingLocation) {
                throwSyntaxError(replacementLocation);
            }
            auto key = processedSite.substr(replacementLocation+opening.size(), endingLocation-1);
            auto val = context[key];
            BLT_TRACE("Key found: %s; val: %s", key.c_str(), val.c_str());
        }
        return processedSite;
    }
    
    void HTMLPage::throwSyntaxError(size_t begin) {
        BLT_ERROR("A syntax error in the (extended) mustache syntax has been detected!");
        BLT_ERROR("Here's what I know:");
        BLT_ERROR("An opening tag was detected at char %d near '%s'",
                  begin, blt::string::trim_copy(processedSite.substr(
                noNeg(begin, 10), begin + 10
        )).c_str());
        throw SyntaxException();
    }
}
