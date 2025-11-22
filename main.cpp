#include "crow_all.h"
#include "database.hpp"

#include <filesystem> 
namespace fs = std::filesystem;

int main() {
    crow::SimpleApp app;

    if (!fs::exists("data")) {
        fs::create_directory("data");
    }

    Database db("data/tasks.db"); 

    CROW_ROUTE(app, "/")([](){
        return crow::mustache::load("index.html").render();
    });

    // Получить все задачи (GET)
    CROW_ROUTE(app, "/api/tasks")
    ([&db](){
        auto tasks = db.getTasks();
        json result = json::array();
        for(auto& t : tasks) {
            result.push_back(t.toJson());
        }
        return crow::response(result.dump());
    });

    // Добавить задачу (POST)
    CROW_ROUTE(app, "/api/tasks").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req){
        try {
            auto x = json::parse(req.body);
            
            if (!x.contains("title")) {
                return crow::response(400, "Missing 'title' field");
            }
            
            db.addTask(x["title"].get<std::string>());
            return crow::response(201);
        } catch (...) {
            return crow::response(400, "Invalid JSON");
        }
    });

    // Переключить статус (PUT)
    CROW_ROUTE(app, "/api/tasks/<int>/toggle").methods(crow::HTTPMethod::PUT)
    ([&db](int id){
        db.toggleTask(id);
        return crow::response(200);
    });

    // Удалить задачу (DELETE)
    CROW_ROUTE(app, "/api/tasks/<int>").methods(crow::HTTPMethod::DELETE)
    ([&db](int id){
        db.deleteTask(id);
        return crow::response(200);
    });

    app.port(18080).multithreaded().run();
}