/*
 * Created by Brett on 25/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/util/memory_reader.h>

#include <cstring>
#include <blt/std/logging.h>
#include <zlib.h>
#include <filesystem>

namespace cs
{
    
    int memory_reader::read(char* buffer, size_t bytes)
    {
        auto end_point = read_point + bytes;
        if (end_point <= size)
        {
            read_amount = bytes;
            std::memcpy(buffer, &memory[read_point], read_amount);
        } else
        {
            //BLT_INFO("Data: EP: %d, RP: %d, BTs: %d, SZ: %d", end_point, read_point, bytes, size);
            // can only read this much data!
            read_amount = size - read_point;
            if (read_amount == 0)
                return 0;
            std::memcpy(buffer, &memory[read_point], read_amount);
        }
        read_point = end_point;
        return 0;
    }
    
    int zip_reader::read(char*, size_t)
    {
        return 0;
    }
    
    zip_reader::~zip_reader()
    {
        delete[] zip_buffer;
    }
    
    void zip_reader::calculateFilesystem()
    {
        if (!std::filesystem::is_directory(path))
        {
            files.push({path, std::filesystem::file_size(path)});
            return;
        }
        std::queue<std::string> directories;
        directories.push(path);
        while (!directories.empty())
        {
            auto front = directories.front();
            directories.pop();
            
            std::filesystem::directory_iterator dir(front);
            for (const auto& p : dir)
            {
                if (p.is_directory())
                    directories.push(p.path());
                else
                    files.push({p.path(), p.file_size()});
            }
        }
    }
}