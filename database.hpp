#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

struct Task {
    int id;
    string title;
    bool completed;

    json toJson() {
        return json{{"id", id}, {"title", title}, {"completed", completed}};
    }
};

class Database {
    sqlite3* db;

public:
    Database(const string& path) {
        if (sqlite3_open(path.c_str(), &db)) {
            cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        } else {
            createTable();
        }
    }

    ~Database() {
        sqlite3_close(db);
    }

    void createTable() {
        const char* sql = "CREATE TABLE IF NOT EXISTS tasks (" \
                          "id INTEGER PRIMARY KEY AUTOINCREMENT," \
                          "title TEXT NOT NULL," \
                          "completed INTEGER DEFAULT 0);";
        char* errMsg;
        sqlite3_exec(db, sql, 0, 0, &errMsg);
    }

    vector<Task> getTasks() {
        vector<Task> tasks;
        string sql = "SELECT id, title, completed FROM tasks;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                tasks.push_back({
                    sqlite3_column_int(stmt, 0),
                    string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))),
                    (bool)sqlite3_column_int(stmt, 2)
                });
            }
        }
        sqlite3_finalize(stmt);
        return tasks;
    }

    void addTask(const string& title) {
        string sql = "INSERT INTO tasks (title) VALUES ('" + title + "');";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    void toggleTask(int id) {
        string sql = "UPDATE tasks SET completed = NOT completed WHERE id = " + to_string(id) + ";";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    void deleteTask(int id) {
        string sql = "DELETE FROM tasks WHERE id = " + to_string(id) + ";";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }
};