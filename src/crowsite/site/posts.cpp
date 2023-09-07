/*
 * Created by Brett on 23/08/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <crowsite/site/posts.h>
#include <crowsite/site/auth.h>
#include <crowsite/utility.h>
#include <crowsite/util/md_to_html.h>
#include <blt/std/logging.h>
#include <crowsite/sql_helper.h>
#include <blt/std/time.h>

namespace cs
{
    sql::database* posts_database;
    
    response_info handleProjectPage(const request_info& req)
    {
        if (req.url_params.contains("post"))
        {
            BLT_TRACE(req.url_params.at("post"));
            sql::statement posts(posts_database, "SELECT file FROM posts WHERE postID=?;");
            posts.set(req.url_params.at("post"), 0);
            posts.execute();
            return {loadMarkdownAsHTML(cs::fs::createDataFilePath(posts.get<std::string>(2)))};
        }
        
        return {""};
    }
    
    void posts_init()
    {
        
        posts_database = new sql::database(cs::fs::createDataFilePath("db/posts.sqlite"));
        
        sql::auto_statement(
                posts_database,
                "CREATE TABLE IF NOT EXISTS posts(postID INTEGER PRIMARY KEY AUTOINCREMENT, date TEXT, title TEXT, file TEXT);"
        );
        
        sql::auto_statement(
                posts_database,
                "CREATE TABLE IF NOT EXISTS modifications(postID INTEGER, date TEXT, FOREIGN KEY(postID) REFERENCES posts(postID));"
        );
        
        sql::auto_statement(
                posts_database,
                "CREATE TABLE IF NOT EXISTS tags(postID INTEGER, tag TEXT, FOREIGN KEY(postID) REFERENCES posts(postID));"
        );
        
        sql::auto_statement(
                posts_database,
                "CREATE TABLE IF NOT EXISTS types(postID INTEGER, type INTEGER, FOREIGN KEY(postID) REFERENCES posts(postID));"
        );
        
    }
    
    void posts_cleanup()
    {
        delete posts_database;
    }
    
    
}