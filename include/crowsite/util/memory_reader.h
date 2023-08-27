#pragma once
/*
 * Created by Brett on 25/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */

#ifndef CROWSITE_MEMORY_READER_H
#define CROWSITE_MEMORY_READER_H

#include <blt/std/filesystem.h>
#include <utility>
#include <queue>

namespace cs
{
    
    class memory_reader : public blt::fs::block_reader
    {
        private:
            char* memory;
            size_t size;
            size_t read_point = 0;
            size_t read_amount = 0;
        public:
            memory_reader(char* mem, size_t s): block_reader(s), memory(mem), size(s)
            {}
            
            inline void reset()
            {
                read_point = 0;
                read_amount = 0;
            }
            
            int read(char* buffer, size_t bytes) final;
            
            inline size_t gcount() final
            {
                return read_amount;
            }
    };
    
    class zip_reader : public blt::fs::block_reader
    {
        private:
            constexpr static size_t BUFFER_SIZE = 1 * 1024 * 1024;
            
            struct file
            {
                std::string path;
                size_t file_size;
            };
            
            std::queue<file> files;
            
            std::string path;
            char* zip_buffer;
            
            size_t read_point = 0;
            size_t write_point = 0;
            size_t read_amount = 0;
            
            void calculateFilesystem();
        
        public:
            /**
             * Creates a zip'd reader from a file or directory. Note: this will read the file or directory as a uncompressed zip file
             * TODO: Compress
             * @param path path to file or directory to stream zip
             */
            explicit zip_reader(std::string path): block_reader(32 * 1024), path(std::move(path)), zip_buffer(new char[BUFFER_SIZE])
            {
                // TODO: all of this later
                // https://en.wikipedia.org/wiki/ZIP_%28file_format%29
                // write the zip magic number
                zip_buffer[write_point++] = 'P';
                zip_buffer[write_point++] = 'K';
                zip_buffer[write_point++] = 0x03;
                zip_buffer[write_point++] = 0x04;
                // zip version
                zip_buffer[write_point++] = 0x03;
                zip_buffer[write_point++] = 0x14;
                // bit flag
                zip_buffer[write_point++] = 0;
                zip_buffer[write_point++] = 0;
                // compression method
                zip_buffer[write_point++] = 0;
                zip_buffer[write_point++] = 0;
                calculateFilesystem();
            }
            
            int read(char* buffer, size_t bytes) final;
            
            inline size_t gcount() final
            {
                return read_amount;
            }
            
            ~zip_reader();
    };
    
}

#endif //CROWSITE_MEMORY_READER_H
