//
// Created by brett on 6/20/23.
//
#include <crowsite/site/web.h>
#include <fstream>
#include <ios>
#include <blt/std/logging.h>

#include <utility>
#include <sstream>
#include <algorithm>

namespace cs {
    
    class LexerSyntaxException : public std::runtime_error {
        public:
            explicit LexerSyntaxException(const std::string& token):
                    std::runtime_error(
                            "Extended-mustache syntax error! An opening '{{' must be closed by '}}'! (near: '" +
                            token + "')"
                    ) {}
    };
    
    class LexerException : public std::runtime_error {
        public:
            explicit LexerException(const std::string& message):
                    std::runtime_error("Extended-mustache syntax processing error! " + message) {}
    };
    
    class SyntaxException : public std::runtime_error {
        public:
            explicit SyntaxException():
                    std::runtime_error(
                            "Extended-mustache syntax error! Static context keys should not contain $"
                    ) {}
    };
    
    class StringLexer {
        private:
            const std::string& str;
            size_t pos = 0;
        public:
            explicit StringLexer(const std::string& str): str(str) {}
            
            inline char nextToken() {
                if (pos >= str.size())
                    return '\0';
                return str[pos++];
            }
            
            inline bool hasTokens() {
                return pos < str.size();
            }
            /**
             * Tries to find the string 'match' and outputs all found characters to 'out'
             * @param match string to match against
             * @param out characters 'tokens' read by the lexer
             * @return true if found false otherwise;
             */
            inline bool findNext(const std::string& match, std::string& out) {
                char c;
                size_t p = 0;
                std::string found;
                while ((c = nextToken())) {
                    // check for match, p should be 0 here!
                    if (c == match[p]) {
                        do {
                            found += c;
                            // emit token
                            out += c;
                            if (found == match){
                                // TODO?
                            }
                            if (c != match[p++]){
                                p = 0;
                                found = "";
                                break;
                            }
                        } while ((c = nextToken()));
                    } else // emit token
                        out += c;
                }
                return false;
            }
    };
    
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
    
    std::string HTMLPage::render(StaticContext& context) {
        std::string processedSiteData = m_SiteData;
        
        std::string buffer;
        
        StringLexer lexer(processedSiteData);
        
        while (lexer.hasTokens()) {
            char c;
            switch ((c = lexer.nextToken())) {
                case '{':
                    // if we are dealing with a mustache template then we should process
                    if ((c = lexer.nextToken()) == '{') {
                        // if it is not the extended syntax we are looking for, skip it as crow will handle it at request time!
                        if ((c = lexer.nextToken()) != '$') {
                            buffer += "{{";
                            buffer += c;
                            break;
                        }
                        std::string tokenString;
                        while ((c = lexer.nextToken())) {
                            if (c == '}') {
                                if (lexer.nextToken() != '}')
                                    throw LexerSyntaxException(tokenString);
                                else {
                                    if (std::find_if(
                                            context.begin(), context.end(),
                                            [&tokenString](auto in) -> bool {
                                                return tokenString == in.first;
                                            }
                                    ) == context.end()) {
                                        // unable to find the token, we should throw an error to tell the user! (or admin in this case)
                                        BLT_WARN("Unable to find token '%s'!", tokenString.c_str());
                                    } else
                                        buffer += context[tokenString];
                                    break;
                                }
                            }
                            tokenString += c;
                        }
                    } else { // otherwise we should write out the characters since this isn't a extended template
                        buffer += '{';
                        buffer += c;
                    }
                    break;
                default:
                    buffer += c;
                    break;
            }
        }
        
        return buffer;
    }
    
    void HTMLPage::resolveResources() {
    
    }
}
