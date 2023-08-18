//
// Created by brett on 17/08/23.
//

#ifndef CROWSITE_SQL_HELPER_H
#define CROWSITE_SQL_HELPER_H

#include <sqlite3.h>
#include <type_traits>

namespace cs::sql
{
    
    static int prepareStatement(sqlite3* db, const std::string& sqlStatement, sqlite3_stmt** ppStmt)
    {
        return sqlite3_prepare_v2(db, sqlStatement.c_str(), static_cast<int>(sqlStatement.size()) + 1, ppStmt, nullptr);
    }
    
    class statement
    {
        private:
            sqlite3_stmt* stmt = nullptr;
            sqlite3* db;
            int err;
        public:
            statement(sqlite3* db, const std::string& statement): db(db)
            {
                err = prepareStatement(db, statement, &stmt);
                if (err)
                    BLT_ERROR("Failed to execute statement, error code %d, error: %s.", err, sqlite3_errstr(err));
            }
            
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
    
}

#endif //CROWSITE_SQL_HELPER_H
