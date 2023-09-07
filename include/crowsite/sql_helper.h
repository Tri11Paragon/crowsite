//
// Created by brett on 17/08/23.
//

#ifndef CROWSITE_SQL_HELPER_H
#define CROWSITE_SQL_HELPER_H

#include <sqlite3.h>
#include <type_traits>
#include <filesystem>
#include "blt/std/assert.h"

namespace cs::sql
{
    
    class sql_error : public std::runtime_error
    {
        public:
            sql_error(const std::string& error): std::runtime_error(error)
            {}
    };
    
    static int prepareStatement(sqlite3* db, const std::string& sqlStatement, sqlite3_stmt** ppStmt)
    {
        return sqlite3_prepare_v2(db, sqlStatement.c_str(), static_cast<int>(sqlStatement.size()) + 1, ppStmt, nullptr);
    }
    
    class database
    {
            friend class statement_base_helper;
        
        private:
            std::string path;
            sqlite3* db;
        public:
            database(const std::string& dbLocation): path(dbLocation)
            {
                std::filesystem::create_directories(dbLocation.substr(0, dbLocation.find_last_of('/') + 1));
                if (int err = sqlite3_open_v2(
                        path.c_str(), &db,
                        SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
                        nullptr
                ) != SQLITE_OK)
                {
                    BLT_FATAL("Unable to create database connection! err %d msg %s", err, sqlite3_errstr(err));
                    std::exit(1);
                }
            }
            
            database(database&& move)
            {
                db = move.db;
                path = move.path;
                move.db = nullptr;
            }
            
            database& operator=(database&& move)
            {
                db = move.db;
                path = move.path;
                move.db = nullptr;
                return *this;
            }
            
            database(const database& copy) = delete;
            
            database& operator=(const database& copy) = delete;
            
            ~database()
            {
                sqlite3_close_v2(db);
            }
    };
    
    class statement_base_helper
    {
        protected:
            sqlite3_stmt* stmt = nullptr;
            int err;
            
            statement_base_helper(const database& db, const std::string& statement, bool throw_errors)
            {
                err = prepareStatement(db.db, statement, &stmt);
                if (err != SQLITE_OK)
                {
                    if (throw_errors)
                    {
                        BLT_THROW(std::runtime_error(
                                          "Failed to prepare statement '" + statement + "', error code " + std::to_string(err) + ", error: " +
                                          sqlite3_errstr(err)));
                    } else
                        BLT_ERROR("Failed to prepare statement, error code %d, error: %s.", err, sqlite3_errstr(err));
                }
            }
    };
    
    class statement : public statement_base_helper
    {
        public:
            statement(statement&& move) = delete;
            
            statement(const statement& copy) = delete;
            
            statement& operator=(statement&& move) = delete;
            
            statement& operator=(const statement& copy) = delete;
            
            statement(const database& db, const std::string& statement, bool throw_errors = true): statement_base_helper(db, statement, throw_errors)
            {}
            
            statement(const database* db, const std::string& statement, bool throw_errors = true): statement_base_helper(*db, statement, throw_errors)
            {}
            
            /**
             * @return true if the last statement failed. Should be checked after construction!
             */
            [[nodiscard]] bool fail() const
            {
                return !(err == SQLITE_OK || err == SQLITE_DONE || err == SQLITE_ROW);
            }
            
            [[nodiscard]] bool hasRow() const
            { return err == SQLITE_ROW; }
            
            [[nodiscard]] bool complete() const
            { return err == SQLITE_DONE; }
            
            [[nodiscard]] int error() const
            {
                return err;
            }
            
            bool execute()
            {
                if (err != SQLITE_ROW)
                    sqlite3_reset(stmt);
                err = sqlite3_step(stmt);
                return !fail();
            }
            
            template<typename T>
            statement* set(const T& t, int column)
            {
                // make api consistent
                column = column + 1;
                if constexpr (std::is_floating_point_v<T>)
                {
                    err = sqlite3_bind_double(stmt, column, t);
                } else if constexpr (std::is_integral_v<T>)
                {
                    if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>)
                        err = sqlite3_bind_int64(stmt, column, t);
                    else
                        err = sqlite3_bind_int(stmt, column, t);
                } else if constexpr (std::is_same_v<T, std::string>)
                {
                    err = sqlite3_bind_text(stmt, column, t.c_str(), -1, nullptr);
                }
                return this;
            }
            
            template<typename T>
            T get(int column)
            {
                if (err != SQLITE_ROW)
                    throw std::runtime_error("Unable to get data as statement didn't return a row!");
                if constexpr (std::is_floating_point_v<T>)
                {
                    return sqlite3_column_double(stmt, column);
                } else if constexpr (std::is_integral_v<T>)
                {
                    if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>)
                        return sqlite3_column_int64(stmt, column);
                    else
                        return sqlite3_column_int(stmt, column);
                } else if constexpr (std::is_same_v<T, std::string>)
                {
                    return std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, column)));
                } else
                {
                    return sqlite3_column_blob(stmt, column);
                }
            }
            
            /**
             * do not run execute before this function. ever.
             * This function will return a default initialized T if execute doesn't return a row.
             */
            template<typename T>
            T executeAndGet(int column)
            {
                execute();
                if (err != SQLITE_ROW)
                    return T{};
                return get<T>(column);
            }
            
            ~statement()
            {
                sqlite3_finalize(stmt);
            }
        
    };
    
    inline void auto_statement(const database* db, const std::string& stmt, bool throw_errors = true){
        statement s(db, stmt, throw_errors);
        if (!s.execute() && throw_errors)
            BLT_THROW(sql_error("Unable to execute statement '" + stmt + "'. Error: " + std::to_string(s.error()) + sqlite3_errstr(s.error())));
    }
    
}

#endif //CROWSITE_SQL_HELPER_H
