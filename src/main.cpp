#include <iostream>
#include <crowsite/crow_includes.h>
#include <blt/std/logging.h>
#include <blt/std/string.h>
#include <blt/profiling/profiler.h>
#include <sstream>
#include <crowsite/utility.h>
#include <crowsite/site/cache.h>
#include <crowsite/beemovie.h>
#include <crowsite/requests/jellyfin.h>
#include <crowsite/requests/curl.h>
#include <blt/parse/argparse.h>
#include <crowsite/site/auth.h>
#include <crow/middlewares/session.h>
#include <crow/middlewares/cookie_parser.h>

using Session = crow::SessionMiddleware<crow::FileStore>;
using CrowApp = crow::App<crow::CookieParser, Session>;

class BLT_CrowLogger : public crow::ILogHandler
{
    public:
        void log(std::string message, crow::LogLevel crow_level) final
        {
            blt::logging::log_level blt_level;
            switch (crow_level)
            {
                case crow::LogLevel::DEBUG:
                    blt_level = blt::logging::log_level::DEBUG;
                    break;
                case crow::LogLevel::INFO:
                    blt_level = blt::logging::log_level::INFO;
                    break;
                case crow::LogLevel::WARNING:
                    blt_level = blt::logging::log_level::WARN;
                    break;
                case crow::LogLevel::ERROR:
                    blt_level = blt::logging::log_level::ERROR;
                    break;
                case crow::LogLevel::CRITICAL:
                    blt_level = blt::logging::log_level::FATAL;
                    break;
            }
            BLT_LOG("Crow: %s", blt_level, message.c_str());
        }
};

struct site_params
{
    CrowApp& app;
    cs::CacheEngine& engine;
    const crow::request& req;
    const std::string& name;
};

inline crow::response redirect(const std::string& loc = "/", int code = 303)
{
    crow::response res(code);
    res.set_header("Location", loc);
    return res;
}

/**
 * Note this function destroys the user's session and any login related cookies!
 */
void destroyUserSession(CrowApp& app, const crow::request& req)
{
    auto& session = app.get_context<Session>(req);
    auto& cookie_context = app.get_context<crow::CookieParser>(req);
    
    session.set("clientID", "");
    session.set("clientToken", "");
    cookie_context.set_cookie("clientID", "");
    cookie_context.set_cookie("clientToken", "");
}

bool checkAndUpdateUserSession(CrowApp& app, const crow::request& req)
{
    auto& session = app.get_context<Session>(req);
    auto& cookie_context = app.get_context<crow::CookieParser>(req);
    
    auto s_clientID = session.get("clientID", "");
    auto s_clientToken = session.get("clientToken", "");
    
    auto c_clientID = cookie_context.get_cookie("clientID");
    auto c_clientToken = cookie_context.get_cookie("clientToken");
    
    if ((!c_clientID.empty() && !c_clientToken.empty()) && (s_clientID != c_clientID || s_clientToken != c_clientToken))
    {
        session.set("clientID", c_clientID);
        session.set("clientToken", c_clientToken);
        return true;
    }
    return false;
}

bool isUserLoggedIn(CrowApp& app, const crow::request& req)
{
    auto& session = app.get_context<Session>(req);
    auto s_clientID = session.get("clientID", "");
    auto s_clientToken = session.get("clientToken", "");
    return cs::isUserLoggedIn(s_clientID, s_clientToken);
}

bool isUserAdmin(CrowApp& app, const crow::request& req)
{
    auto& session = app.get_context<Session>(req);
    auto s_clientID = session.get("clientID", "");
    return cs::isUserAdmin(cs::getUserFromID(s_clientID));
}

crow::response handle_root_page(const site_params& params)
{
    //auto page = crow::mustache::load("index.html"); //
    //return "<html><head><title>Hello There</title></head><body><h1>Suck it " + name + "</h1></body></html>";
//                BLT_TRACE(req.body);
//                for (const auto& h : req.headers)
//                    BLT_TRACE("Header: %s = %s", h.first.c_str(), h.second.c_str());
//                BLT_TRACE(req.raw_url);
//                BLT_TRACE(req.url);
//                BLT_TRACE(req.remote_ip_address);
//                for (const auto& v : req.url_params.keys())
//                    BLT_TRACE("URL: %s = %s", v.c_str(), req.url_params.get(v));
    if (params.name.ends_with(".html"))
    {
        checkAndUpdateUserSession(params.app, params.req);
        auto& session = params.app.get_context<Session>(params.req);
        auto s_clientID = session.get("clientID", "");
        auto s_clientToken = session.get("clientToken", "");
        auto user_perms = cs::getUserPermissions(cs::getUserFromID(s_clientID));
        
        crow::mustache::context ctx;
        
        // pass perms in
        if (user_perms & cs::PERM_ADMIN)
            ctx["_admin"] = true;
        
        if (cs::isUserLoggedIn(s_clientID, s_clientToken))
        {
            ctx["_logged_in"] = true;
        } else {
            ctx["_not_logged_in"] = true;
        }
        
        // we don't want to pass all get parameters to the context to prevent leaking information
        auto referer = params.req.url_params.get("referer");
        if (referer)
            ctx["referer"] = referer;
        auto page = crow::mustache::compile(params.engine.fetch(params.name));
        return page.render(ctx);
    }
    
    return params.engine.fetch("default.html");
}

crow::response handle_auth_page(const site_params& params)
{
    if (isUserAdmin(params.app, params.req))
        return redirect("/login.html");
    
    
    
    return handle_root_page(params);
}

int main(int argc, const char** argv)
{
    blt::logging::setLogOutputFormat(
            "\033[94m[${{FULL_TIME}}]${{RC}} ${{LF}}[${{LOG_LEVEL}}]${{RC}} \033[35m(${{FILE}}:${{LINE}})${{RC}} ${{CNR}}${{STR}}${{RC}}\n"
    );
    cs::requests::init();
    
    blt::arg_parse parser;
    parser.addArgument(blt::arg_builder("--tests").setAction(blt::arg_action_t::STORE_TRUE).build());
    parser.addArgument(blt::arg_builder({"--port", "-p"}).setDefault(8080).build());
    parser.addArgument(blt::arg_builder("token").build());
    auto args = parser.parse_args(argc, argv);
    cs::jellyfin::setToken(blt::arg_parse::get<std::string>(args["token"]));
    cs::jellyfin::processUserData();
    cs::auth::init();
    
    BLT_INFO("Starting site %s.", SITE_NAME);
    crow::mustache::set_global_base(SITE_FILES_PATH);
    static BLT_CrowLogger bltCrowLogger{};
    crow::logger::setHandler(&bltCrowLogger);
    
    const auto session_age = 24 * 60 * 60;
    const auto cookie_age = 180 * 24 * 60 * 60;
    
    BLT_INFO("Init Crow with compression and logging enabled!");
    CrowApp app{Session{
            // customize cookies
            crow::CookieParser::Cookie("session").max_age(session_age).path("/"),
            // set session id length (small value only for demonstration purposes)
            16,
            // init the store
            crow::FileStore{std::string(SITE_FILES_PATH) + "/data/session", session_age}}};
    app.use_compression(crow::compression::GZIP);
    app.loglevel(crow::LogLevel::WARNING);
    
    BLT_INFO("Creating static context");
    
    cs::StaticContext context;
    context["SITE_TITLE"] = SITE_TITLE;
    context["SITE_NAME"] = SITE_NAME;
    context["SITE_VERSION"] = SITE_VERSION;
    context["BEE_MOVIE"] = beemovie_script;
    context["SITE_BACKGROUND"] = "/static/images/backgrounds/2023-05-26_23.18.23.png";
    
    BLT_INFO("Starting cache engine");
    
    cs::CacheSettings settings;
    cs::CacheEngine engine(context, settings);
    
    BLT_INFO("Creating routes");
    
    CROW_ROUTE(app, "/favicon.ico")(
            [](crow::response& local_fav_res) {
                local_fav_res.compressed = false;
                local_fav_res.set_static_file_info_unsafe(cs::fs::createStaticFilePath("images/favicon.ico"));
                local_fav_res.set_header("content-type", "image/x-icon");
                local_fav_res.end();
            }
    );
    
    CROW_ROUTE(app, "/login.html")(
            [&app, &engine](const crow::request& req) -> crow::response {
                if (isUserLoggedIn(app, req))
                    return redirect("/");
                return handle_root_page({app, engine, req, "login.html"});
            }
    );
    
    CROW_ROUTE(app, "/logout.html")(
            [&app](const crow::request& req) -> crow::response {
                destroyUserSession(app, req);
                return redirect("/");
            }
    );
    
    CROW_ROUTE(app, "/<string>")(
            [&app, &engine](const crow::request& req, const std::string& name) -> crow::response {
                return handle_root_page({app, engine, req, name});
            }
    );
    
    CROW_ROUTE(app, "/res/login").methods(crow::HTTPMethod::POST)(
            [&app](const crow::request& req) {
                cs::parser::Post pp(req.body);
                auto& session = app.get_context<Session>(req);
                
                std::string user_agent;
                
                for (const auto& h : req.headers)
                    if (h.first == "User-Agent")
                    {
                        user_agent = h.second;
                        break;
                    }
                
                // either redirect to clear the form if failed or pass user to index
                if (cs::checkUserAuthorization(pp))
                {
                    cs::cookie_data data = cs::createUserAuthTokens(pp, user_agent);
                    if (!cs::storeUserData(pp["username"], user_agent, data))
                    {
                        BLT_ERROR("Failed to update user data");
                        return redirect("login.html");
                    }
                    
                    session.set("clientID", data.clientID);
                    session.set("clientToken", data.clientToken);
                    if (pp.hasKey("remember_me") && pp["remember_me"][0] == 'T')
                    {
                        auto& cookie_context = app.get_context<crow::CookieParser>(req);
                        cookie_context.set_cookie("clientID", data.clientID).path("/").max_age(cookie_age);
                        cookie_context.set_cookie("clientToken", data.clientToken).path("/").max_age(cookie_age);
                    }
                    return redirect(pp.hasKey("referer") ? pp["referer"] : "/");
                } else
                    return redirect("login.html");
            }
    );
    
    CROW_ROUTE(app, "/")(
            [&engine, &app](const crow::request& req) {
                return handle_root_page({app, engine, req, "index.html"});
            }
    );
    
    CROW_CATCHALL_ROUTE(app)(
            [&engine]() {
                return engine.fetch("default.html");
            }
    );
    
    app.tick(
            std::chrono::seconds(1), [&app, &args]() -> void {
                if (!args.contains("tests"))
                    return;
                static uint64_t timer = 0;
                const uint64_t wait = 10;
                timer++;
                if (timer > wait)
                {
                    app.stop();
                }
            }
    );
    auto port = blt::arg_parse::get_cast<int32_t>(args["port"]);
    BLT_INFO("Starting Crow website on port %d", port);
    app.port(port).multithreaded().run();
    
    cs::requests::cleanup();
    cs::auth::cleanup();
    
    return 0;
}
