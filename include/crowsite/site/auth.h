//
// Created by brett on 16/08/23.
//

#ifndef CROWSITE_AUTH_H
#define CROWSITE_AUTH_H

#include "crowsite/utility.h"
#include <string>

namespace cs {
    
    constexpr uint32_t PERM_ADMIN = 0x1;
    constexpr uint32_t PERM_READ_FILES = 0x2;
    constexpr uint32_t PERM_WRITE_FILES = 0x4;
    constexpr uint32_t PERM_CREATE_POSTS = 0x8;
    constexpr uint32_t PERM_CREATE_COMMENTS = 0x10;
    constexpr uint32_t PERM_CREATE_SHARES = 0x20;
    constexpr uint32_t PERM_EDIT_POSTS = 0x40;
    constexpr uint32_t PERM_EDIT_COMMENTS = 0x80;
    
    constexpr uint32_t PERM_DEFAULT = PERM_READ_FILES | PERM_CREATE_COMMENTS;
    
    namespace auth {
        void init();
        void cleanup();
    }
    
    struct cookie_data {
        std::string clientID;
        std::string clientToken;
    };
    
    /**
     *  An interface function which is used to validate login information provided as post data. Is is up to the caller
     *  to inform the user of the clientID and clientToken, along with the auth system of these values.
     *  Which is to say that this function is purely for auth.
     *
     * @param postData post data container class. This function checks for the existence of "username" and "password" in postData
     * @related createUserAuthTokens(...)
     * @related storeUserData(...)
     * @return true if user is valid and authorized, false otherwise (including if "username" || "password" does not exist).
     */
    bool checkUserAuthorization(cs::parser::Post& postData);
    
    /**
     * Generates a clientID (UUIDv5 based on user-agent and username) along with a unique high security (512 bit) base64 encoded token string
     * @param postData post data including a "username"
     * @param useragent user agent of the requesting client
     * @return cookie_data structure containing clientId and clientToken
     */
    cookie_data createUserAuthTokens(cs::parser::Post& postData, const std::string& useragent);
    
    /**
     * Informs the internal auth database of a successfully login attempt, updating the internal storage of the clientID -> Token.
     * Username is passed directly as this function should only be called after checkUserAuthorization(...) returns true.
     * @param username username of the user.
     * @param useragent user-agent of the user
     * @param tokens generated client tokens
     * @return false if something failed (error will be logged!)
     * @related createUserAuthTokens(...)
     * @related checkUserAuthorization(...)
     */
    bool storeUserData(const std::string& username, const std::string& useragent, const cookie_data& tokens);
    
    bool isUserLoggedIn(const std::string& clientID, const std::string& token);
    
    std::string getUserFromID(const std::string& clientID);
    bool isUserAdmin(const std::string& username);
    uint32_t getUserPermissions(const std::string& username);
    
}

#endif //CROWSITE_AUTH_H
