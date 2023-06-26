#include <iostream>
#include <crowsite/crow_includes.h>
#include <blt/std/logging.h>
#include <crowsite/utility.h>
#include <crowsite/site/cache.h>

int main() {
    BLT_INFO("Starting site %s.", SITE_NAME);
    crow::mustache::set_global_base(SITE_FILES_PATH);
    
    BLT_INFO("Init Crow with compression and logging enabled!");
    crow::SimpleApp app;
    app.use_compression(crow::compression::GZIP);
    app.loglevel(crow::LogLevel::INFO);
    
    BLT_INFO("Creating static context");
    
    cs::StaticContext context;
    context["SITE_TITLE"] = SITE_TITLE;
    context["SITE_NAME"] = SITE_NAME;
    context["SITE_VERSION"] = SITE_VERSION;
    
    BLT_INFO("Starting cache engine");
    
    cs::CacheSettings settings;
    cs::CacheEngine engine(context, settings);
    
    BLT_INFO("Creating routes");
    
    CROW_ROUTE(app, "/favicon.ico")([](crow::response& local_fav_res) {
        local_fav_res.compressed = false;
        local_fav_res.set_static_file_info_unsafe(cs::fs::createStaticFilePath("images/favicon.ico"));
        local_fav_res.set_header("content-type", "image/x-icon");
        local_fav_res.end();
    });
    
    CROW_ROUTE(app, "/<string>")(
            [&](const std::string& name) {
                //auto page = crow::mustache::load("index.html"); //
                //return "<html><head><title>Hello There</title></head><body><h1>Suck it " + name + "</h1></body></html>";
                crow::mustache::context ctx({{"person", name}});
                auto user_page = crow::mustache::compile(engine.fetch("index.html"));
                
                //BLT_TRACE(page);
                
                return user_page.render(ctx);
            }
    );
    
    CROW_ROUTE(app, "/req/posta.html").methods(crow::HTTPMethod::POST)(
            [](const crow::request& req) {
                cs::parser::Post pp(req.body);
                
                return "Portabella Mushrooms! " + pp.dump();
            }
    );
    
    CROW_CATCHALL_ROUTE(app)(
            []() {
                return "World's Sexyest Man";
            }
    );
    
    app.port(8080).multithreaded().run();
    
    return 0;
}
