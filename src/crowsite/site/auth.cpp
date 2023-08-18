//
// Created by brett on 16/08/23.
//
#include <crowsite/site/auth.h>
#include <crowsite/config.h>
#include <crowsite/requests/jellyfin.h>
#include "blt/std/logging.h"
#include "blt/std/uuid.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <crowsite/sql_helper.h>
#include <random>
#include <filesystem>

using namespace blt;

namespace cs
{
    sqlite3* user_database;
    
    // https://stackoverflow.com/questions/5288076/base64-encoding-and-decoding-with-openssl
    
    char* base64(const unsigned char* input, int length)
    {
        const auto pl = 4 * ((length + 2) / 3);
        auto output = reinterpret_cast<char*>(calloc(pl + 1, 1)); //+1 for the terminating null that EVP_EncodeBlock adds on
        const auto ol = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(output), input, length);
        if (pl != ol)
        {
            std::cerr << "Whoops, encode predicted " << pl << " but we got " << ol << "\n";
        }
        return output;
    }
    
    unsigned char* decode64(const char* input, int length)
    {
        const auto pl = 3 * length / 4;
        auto output = reinterpret_cast<unsigned char*>(calloc(pl + 1, 1));
        const auto ol = EVP_DecodeBlock(output, reinterpret_cast<const unsigned char*>(input), length);
        if (pl != ol)
        {
            std::cerr << "Whoops, decode predicted " << pl << " but we got " << ol << "\n";
        }
        return output;
    }
    
    bool checkUserAuthorization(cs::parser::Post& postData)
    {
        // javascript should make sure we don't send post requests without information
        // this way it can be interactive
        if (!postData.hasKey("username") || !postData.hasKey("password"))
            return false;
        auto auth = jellyfin::authenticateUser(postData["username"], postData["password"]);
        
        return auth == jellyfin::auth_response::AUTHORIZED;
    }
    
    cookie_data createUserAuthTokens(parser::Post& postData, const std::string& useragent)
    {
        cookie_data cookieOut;
        cookieOut.clientID = uuid::toString(uuid::genV5(postData["username"] + "::" + useragent));
        
        std::string token;
        
        std::random_device rd;
        std::seed_seq seed{rd(), rd(), rd(), rd()};
        std::mt19937_64 gen(seed);
        std::uniform_int_distribution<int> charDist(0, 92);
        
        for (int i = 0; i < SHA512_DIGEST_LENGTH * 8; i++)
            token += char(33 + charDist(gen));
        
        unsigned char hash[SHA512_DIGEST_LENGTH + 1];
        hash[SHA512_DIGEST_LENGTH] = '\0';
        
        SHA512(reinterpret_cast<const unsigned char*>(token.c_str()), token.size(), hash);
        
        auto b64str = base64(hash, SHA512_DIGEST_LENGTH);
        
        cookieOut.clientToken = std::string(reinterpret_cast<const char*>(b64str));
        
        free(b64str);
        
        return cookieOut;
    }
    
    bool storeUserData(const std::string& username, const std::string& useragent, const cookie_data& tokens)
    {
        sql::statement insertStmt{
                user_database,
                "INSERT OR REPLACE INTO user_sessions (clientID, username, useragent, token) VALUES (?, ?, ?, ?);"
        };
        
        if (insertStmt.fail())
            return false;
        
        insertStmt.set(tokens.clientID, 0);
        insertStmt.set(username, 1);
        insertStmt.set(useragent, 2);
        insertStmt.set(tokens.clientToken, 3);
        
        if (!insertStmt.execute())
            return false;
        
        sql::statement insertAuth {
            user_database,
            "INSERT OR REPLACE INTO user_permissions (username, permission) VALUES (?, ?);"
        };
        if (insertAuth.fail())
            return false;
        insertStmt.set(username, 0);
        insertStmt.set(PERM_DEFAULT | (jellyfin::getUserData(username).isAdmin ? PERM_ADMIN : 0), 1);
        
        if (!insertAuth.execute())
            return false;
        
        return true;
    }
    
    bool isUserLoggedIn(const std::string& clientID, const std::string& token)
    {
        sql::statement stmt {
            user_database,
            "SELECT username FROM user_sessions WHERE clientID='?' AND token='?';"
        };
        if (stmt.fail())
            return false;
        stmt.set(clientID, 0);
        stmt.set(token, 1);
        stmt.execute();
        return stmt.hasRow();
    }
    
    bool isUserAdmin(const std::string& username)
    {
        return getUserPermissions(username) & PERM_ADMIN;
    }
    
    std::string getUserFromID(const std::string& clientID)
    {
        sql::statement stmt {
            user_database,
            "SELECT username FROM user_sessions WHERE clientID='?';"
        };
        if (stmt.fail())
            return "";
        stmt.set(clientID, 0);
        return stmt.executeAndGet<std::string>(0);
    }
    
    uint32_t getUserPermissions(const std::string& username)
    {
        sql::statement stmt {
            user_database,
            "SELECT permission FROM user_permissions WHERE username='?';"
        };
        if (stmt.fail())
            return 0;
        stmt.set(username, 0);
        return static_cast<uint32_t>(stmt.executeAndGet<int32_t>(0));
    }
    
    void auth::init()
    {
        // TODO: proper multithreading
        auto path = (std::string(SITE_FILES_PATH) + "/data/db/");
        auto dbname = "users.sqlite";
        auto full_path = path + dbname;
        BLT_TRACE("Using %s for users database", full_path.c_str());
        std::filesystem::create_directories(path);
        if (int err = sqlite3_open_v2(full_path.c_str(), &user_database,
                                      SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
                                      nullptr
        ) != SQLITE_OK)
        {
            BLT_FATAL("Unable to create database connection! err %d msg %s", err, sqlite3_errstr(err));
            std::exit(1);
        }
        
        sql::statement v {
                user_database,
                "SELECT SQLITE_VERSION()"
        };
        
        if (v.fail())
            BLT_WARN("Failed to create statement with error code: %d msg: %s", v.error(), sqlite3_errstr(v.error()));
        
        if (!v.execute())
            BLT_WARN("Failed to execute statement with error code: %d msg: %s", v.error(), sqlite3_errstr(v.error()));
        
        BLT_INFO("SQLite Version: %s", v.get<std::string>(0).c_str());
        
        sql::statement tableStmt{
                user_database,
                "CREATE TABLE IF NOT EXISTS user_sessions (clientID VARCHAR(36), username TEXT, useragent TEXT, token TEXT, PRIMARY KEY(clientID));"
        };
        
        if (tableStmt.fail() || !tableStmt.execute())
            BLT_ERROR("Failed to execute user_sessions table creation! %d : %s", tableStmt.error(), sqlite3_errstr(tableStmt.error()));
        
        sql::statement permsStmt{
                user_database,
                "CREATE TABLE IF NOT EXISTS user_permissions (username TEXT, permission INT, PRIMARY KEY(username));"
        };
        
        if (permsStmt.fail() || !permsStmt.execute())
            BLT_ERROR("Failed to execute user_permissions table creation! %d : %s", permsStmt.error(), sqlite3_errstr(permsStmt.error()));
    }
    
    void auth::cleanup()
    {
        sqlite3_close_v2(user_database);
    }
}