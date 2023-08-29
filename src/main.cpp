#include <crowsite/crow_includes.h>
#include <blt/std/logging.h>
#include <crowsite/utility.h>
#include <crowsite/site/cache.h>
#include <crowsite/beemovie.h>
#include <crowsite/requests/jellyfin.h>
#include <crowsite/requests/curl.h>
#include <blt/parse/argparse.h>
#include <crowsite/site/auth.h>
#include <crowsite/site/home.h>
#include <crowsite/site/posts.h>
#include <crowsite/util/crow_log.h>
#include <crowsite/util/crow_conversion.h>

#define CS_SESSION cs::checkAndUpdateUserSession(app, req); \
                    auto& session = app.get_context<Session>(req); \
                    auto s_clientID = session.get("clientID", ""); \
                    auto s_clientToken = session.get("clientToken", ""); \

int main(int argc, const char** argv)
{
    blt::logging::setLogOutputFormat(
            "\033[94m[${{FULL_TIME}}]${{RC}} ${{LF}}[${{LOG_LEVEL}}]${{RC}} \033[35m(${{FILE}}:${{LINE}})${{RC}} ${{CNR}}${{STR}}${{RC}}\n"
    );
    
    cs::requests::init();
    
    blt::arg_parse parser;
    parser.addArgument(blt::arg_builder("--tests").setAction(blt::arg_action_t::STORE_TRUE).build());
    parser.addArgument(blt::arg_builder({"--port", "-p"}).setDefault(8080).build());
    parser.addArgument(blt::arg_builder("token").setRequired().build());
    auto args = parser.parse_args(argc, argv);
    cs::jellyfin::setToken(blt::arg_parse::get<std::string>(args["token"]));
    cs::jellyfin::processUserData();
    cs::auth::init();
    
    BLT_INFO("Starting site %s.", SITE_NAME);
    crow::mustache::set_global_base(CROWSITE_FILES_PATH);
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
            crow::FileStore{std::string(CROWSITE_FILES_PATH) + "/data/session", session_age}}};
    app.use_compression(crow::compression::GZIP);
    app.loglevel(crow::LogLevel::WARNING);
    
    BLT_INFO("Creating static context");
    
    cs::context static_context;
    static_context["SITE_TITLE"] = SITE_TITLE;
    static_context["SITE_NAME"] = SITE_NAME;
    static_context["SITE_VERSION"] = SITE_VERSION;
    static_context["BEE_MOVIE"] = beemovie_script;
    static_context["SITE_BACKGROUND"] = "/static/images/backgrounds/2023-05-26_23.18.23.png";
    static_context["MENU_BAR_COLOR"] = "#335";
    static_context["MENU_BAR_HOVER"] = "#223";
    static_context["MENU_BAR_ACTIVE"] = "#7821be";
    
    BLT_INFO("Starting cache engine");
    
    cs::CacheSettings settings;
    cs::CacheEngine engine(static_context, settings);
    
    cs::posts_init();
    
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
                if (cs::isUserLoggedIn(app, req))
                    return cs::redirect("/");
                return cs::handle_root_page({app, engine, req, "login.html"});
            }
    );
    
    CROW_ROUTE(app, "/logout.html")(
            [&app](const crow::request& req) -> crow::response {
                cs::destroyUserSession(app, req);
                return cs::redirect("/");
            }
    );
    
    CROW_ROUTE(app, "/<string>")(
            [&app, &engine](const crow::request& req, const std::string& name) -> crow::response {
                return cs::handle_root_page({app, engine, req, name});
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
                
                // either cs::redirect to clear the form if failed or pass user to index
                if (cs::checkUserAuthorization(pp))
                {
                    cs::cookie_data data = cs::createUserAuthTokens(pp, user_agent);
                    if (!cs::storeUserData(pp["username"], user_agent, data))
                    {
                        BLT_ERROR("Failed to update user data");
                        return cs::redirect("login.html");
                    }
                    
                    session.set("clientID", data.clientID);
                    session.set("clientToken", data.clientToken);
                    if (pp.hasKey("remember_me") && pp["remember_me"][0] == 'T')
                    {
                        auto& cookie_context = app.get_context<crow::CookieParser>(req);
                        cookie_context.set_cookie("clientID", data.clientID).path("/").max_age(cookie_age);
                        cookie_context.set_cookie("clientToken", data.clientToken).path("/").max_age(cookie_age);
                    }
                    return cs::redirect(pp.hasKey("referer") ? pp["referer"] : "/");
                } else
                    return cs::redirect("login.html");
            }
    );
    
    CROW_ROUTE(app, "/projects/<path>")(
            [&engine, &app](const crow::request& req, const std::string& path) {
                CS_SESSION;
                
                return toResponse(
                        cs::handleProjectPage({req.raw_url, s_clientID, s_clientToken, path, cs::toQueryString(req.url_params.getValues()), engine}));
            }
    );
    
    CROW_ROUTE(app, "/projects/")(
            [&engine, &app](const crow::request& req) {
                CS_SESSION;
                
                return toResponse(
                        cs::handleProjectPage(
                                {req.raw_url, s_clientID, s_clientToken, "index.html", cs::toQueryString(req.url_params.getValues()), engine}
                        ));
            }
    );
    
    CROW_ROUTE(app, "/")(
            [&engine, &app](const crow::request& req) {
                return cs::handle_root_page({app, engine, req, "index.html"});
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
    
    cs::posts_cleanup();
    
    cs::requests::cleanup();
    cs::auth::cleanup();
    
    return 0;
}
