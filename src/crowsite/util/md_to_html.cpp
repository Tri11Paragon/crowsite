/*
 * Created by Brett on 27/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/util/md_to_html.h>
#include <md4c-html.h>
#include <sstream>
#include <fstream>
#include <cstring>

namespace cs {
    
    class string_buffer
    {
        private:
            constexpr static size_t initial_size = 512;
            char* buffer;
            size_t buffer_size = initial_size;
            size_t used_size = 0;
            
            void expand()
            {
                size_t new_size = buffer_size * 2;
                char* tmp = new char[new_size];
                for (size_t i = 0; i < used_size; i++)
                    tmp[i] = buffer[i];
                delete[] buffer;
                buffer = tmp;
                buffer_size = new_size;
            }
        
        public:
            string_buffer(): buffer(new char[initial_size])
            {}
            
            ~string_buffer()
            { delete[] buffer; }
            
            void append(const MD_CHAR* text, MD_SIZE size)
            {
                if (used_size + size >= buffer_size)
                    expand();
                std::memcpy(&buffer[used_size], text, size);
                used_size += size;
            }
            
            std::string str()
            {
                std::string out (buffer, used_size);
                return out;
            }
    };
    
    void process_output(const MD_CHAR* text, MD_SIZE size, void* data)
    {
        reinterpret_cast<string_buffer*>(data)->append(text, size);
    }
    
    std::string loadMarkdownAsHTML(const std::string& path)
    {
        std::string mdData;
        string_buffer output;
        {
            std::ifstream file(path);
            std::stringstream stream;
            stream << file.rdbuf();
            mdData = stream.str();
        }
        const unsigned int parse_flags =
                MD_FLAG_TABLES | MD_FLAG_TASKLISTS | MD_FLAG_STRIKETHROUGH | MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_PERMISSIVEEMAILAUTOLINKS
                | MD_FLAG_PERMISSIVEWWWAUTOLINKS | MD_FLAG_LATEXMATHSPANS | MD_FLAG_WIKILINKS | MD_FLAG_UNDERLINE;
        md_html(mdData.c_str(), mdData.size(), process_output, &output, parse_flags, 0);
        return output.str();
    }
}