#include "crow_all.h"
#include "database.hpp"
#include <filesystem> 
#include <fstream>

namespace fs = std::filesystem;

int main() {
    crow::SimpleApp app;

    if (!fs::exists("data")) fs::create_directory("data");
    Database db("data/tasks.db"); 

    CROW_ROUTE(app, "/")([](){ return crow::mustache::load("index.html").render(); });

    CROW_ROUTE(app, "/static/<string>")
    ([](crow::response& res, string filename){
        string path = "static/" + filename;
        ifstream in(path, ios::in);
        if (in) {
            ostringstream contents;
            contents << in.rdbuf();
            res.write(contents.str());
            if (filename.find(".css") != string::npos) res.add_header("Content-Type", "text/css");
            res.end();
        } else { res.code = 404; res.end(); }
    });

    // GET TASKS
    CROW_ROUTE(app, "/api/tasks")
    ([&db](){
        auto tasks = db.getTasks();
        json result = json::array();
        for(auto& t : tasks) result.push_back(t.toJson());
        return crow::response(result.dump());
    });

    // ADD TASK
    CROW_ROUTE(app, "/api/tasks").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req){
        try {
            auto x = json::parse(req.body);
            if (!x.contains("title")) return crow::response(400);
            db.addTask(x["title"].get<std::string>());
            return crow::response(201);
        } catch (...) { return crow::response(400); }
    });

    // EDIT TITLE
    CROW_ROUTE(app, "/api/tasks/<int>").methods(crow::HTTPMethod::PUT)
    ([&db](const crow::request& req, int id){
        try {
            auto x = json::parse(req.body);
            if (x.contains("title")) {
                db.updateTaskTitle(id, x["title"].get<std::string>());
            }
            return crow::response(200);
        } catch (...) { return crow::response(400); }
    });

    // UPDATE STATUS (Drag & Drop или Select)
    // Body: { "status": "done" }
    CROW_ROUTE(app, "/api/tasks/<int>/status").methods(crow::HTTPMethod::PUT)
    ([&db](const crow::request& req, int id){
        try {
            auto x = json::parse(req.body);
            if (x.contains("status")) {
                db.updateStatus(id, x["status"].get<std::string>());
            }
            return crow::response(200);
        } catch (...) { return crow::response(400); }
    });

    // TOGGLE TIMER (Play/Pause)
    CROW_ROUTE(app, "/api/tasks/<int>/timer").methods(crow::HTTPMethod::POST)
    ([&db](int id){
        db.toggleTimer(id);
        return crow::response(200);
    });

    // DELETE
    CROW_ROUTE(app, "/api/tasks/<int>").methods(crow::HTTPMethod::DELETE)
    ([&db](int id){ db.deleteTask(id); return crow::response(200); });

    // DELETE COMPLETED
    CROW_ROUTE(app, "/api/tasks/completed").methods(crow::HTTPMethod::DELETE)
    ([&db](){ db.deleteCompleted(); return crow::response(200); });

    app.port(18080).multithreaded().run();
}