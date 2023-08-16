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

class BLT_CrowLogger : public crow::ILogHandler
{
    public:
        void log(std::string message, crow::LogLevel crow_level) final
        {
            blt::logging::log_level blt_level;
            switch (crow_level){
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


int main(int argc, const char** argv)
{
    blt::logging::setLogOutputFormat(
            "\033[94m[${{FULL_TIME}}]${{RC}} ${{LF}}[${{LOG_LEVEL}}]${{RC}} \033[35m(${{FILE}}:${{LINE}})${{RC}} ${{CNR}}${{STR}}${{RC}}\n"
    );
    cs::requests::init();
    
    blt::arg_parse parser;
    parser.addArgument(blt::arg_builder("token").build());
    parser.addArgument(blt::arg_builder("user").build());
    parser.addArgument(blt::arg_builder("pass").build());
    auto args = parser.parse_args(argc, argv);
    cs::jellyfin::setToken(blt::arg_parse::get<std::string>(args["token"]));
    cs::jellyfin::processUserData();
    
    cs::jellyfin::authenticateUser(blt::arg_parse::get<std::string>(args["user"]), blt::arg_parse::get<std::string>(args["pass"]));
    
    BLT_INFO("Starting site %s.", SITE_NAME);
    crow::mustache::set_global_base(SITE_FILES_PATH);
    static BLT_CrowLogger bltCrowLogger{};
    crow::logger::setHandler(&bltCrowLogger);
    
    BLT_INFO("Init Crow with compression and logging enabled!");
    crow::SimpleApp app;
    app.use_compression(crow::compression::GZIP);
    app.loglevel(crow::LogLevel::INFO);
    
    BLT_INFO("Creating static context");
    
    cs::StaticContext context;
    context["SITE_TITLE"] = SITE_TITLE;
    context["SITE_NAME"] = SITE_NAME;
    context["SITE_VERSION"] = SITE_VERSION;
    context["BEE_MOVIE"] = beemovie_script;
    
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
    
    CROW_ROUTE(app, "/<string>")(
            [&](const crow::request& req, const std::string& name) -> crow::response {
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
                if (name.ends_with(".html"))
                    return {engine.fetch(name)};
                
                crow::mustache::context ctx({{"person", name}});
                auto user_page = crow::mustache::compile(engine.fetch("index.html"));
                
                //BLT_TRACE(page);
                
                return user_page.render(ctx);
            }
    );
    
    CROW_ROUTE(app, "/res/login").methods(crow::HTTPMethod::POST)(
            [](const crow::request& req) {
                cs::parser::Post pp(req.body);
                
                return "Portabella Mushrooms! " + pp.dump();
            }
    );
    
    CROW_ROUTE(app, "/")(
            [&engine]() {
                return engine.fetch("home.html");
            }
    );
    
    CROW_CATCHALL_ROUTE(app)(
            [&engine]() {
                return engine.fetch("default.html");
            }
    );
    
    app.port(8080).multithreaded().run();
    
    cs::requests::cleanup();
    
    return 0;
}
