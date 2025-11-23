#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

struct Task {
    int id;
    string title;
    string status;      // "todo", "in_progress", "done"
    long long time_spent;
    long long start_timestamp;

    json toJson() {
        return json{
            {"id", id}, 
            {"title", title}, 
            {"status", status},
            {"time_spent", time_spent},
            {"start_timestamp", start_timestamp}
        };
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
                          "status TEXT DEFAULT 'todo'," \
                          "time_spent INTEGER DEFAULT 0," \
                          "start_timestamp INTEGER DEFAULT 0);";
        char* errMsg;
        sqlite3_exec(db, sql, 0, 0, &errMsg);
    }

    vector<Task> getTasks() {
        vector<Task> tasks;
        string sql = "SELECT id, title, status, time_spent, start_timestamp FROM tasks ORDER BY id DESC;";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                tasks.push_back({
                    sqlite3_column_int(stmt, 0),
                    string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))),
                    string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))),
                    sqlite3_column_int64(stmt, 3),
                    sqlite3_column_int64(stmt, 4)
                });
            }
        }
        sqlite3_finalize(stmt);
        return tasks;
    }

    void addTask(const string& title) {
        string sql = "INSERT INTO tasks (title, status) VALUES ('" + title + "', 'todo');";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    void updateTaskTitle(int id, const string& newTitle) {
        string sql = "UPDATE tasks SET title = '" + newTitle + "' WHERE id = " + to_string(id) + ";";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    void updateStatus(int id, const string& status) {
        if (status == "done") {
            stopTimer(id);
        }
        string sql = "UPDATE tasks SET status = '" + status + "' WHERE id = " + to_string(id) + ";";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    void toggleTimer(int id) {
        string query = "SELECT time_spent, start_timestamp, status FROM tasks WHERE id = " + to_string(id) + ";";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                long long spent = sqlite3_column_int64(stmt, 0);
                long long start = sqlite3_column_int64(stmt, 1);
                string status = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
                
                long long now = std::time(nullptr);

                if (start == 0) {
                    string sql = "UPDATE tasks SET start_timestamp = " + to_string(now) + 
                                 ", status = 'in_progress' WHERE id = " + to_string(id) + ";";
                    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
                } else {
                    long long delta = now - start;
                    string sql = "UPDATE tasks SET time_spent = " + to_string(spent + delta) + 
                                 ", start_timestamp = 0 WHERE id = " + to_string(id) + ";";
                    sqlite3_exec(db, sql.c_str(), 0, 0, 0);
                }
            }
        }
        sqlite3_finalize(stmt);
    }

    void stopTimer(int id) {
        toggleTimer(id);
        long long now = std::time(nullptr);
        string sql = "UPDATE tasks SET time_spent = time_spent + (" + to_string(now) + " - start_timestamp), " \
                     "start_timestamp = 0 " \
                     "WHERE id = " + to_string(id) + " AND start_timestamp > 0;";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    void deleteTask(int id) {
        string sql = "DELETE FROM tasks WHERE id = " + to_string(id) + ";";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }

    void deleteCompleted() {
        string sql = "DELETE FROM tasks WHERE status = 'done';";
        sqlite3_exec(db, sql.c_str(), 0, 0, 0);
    }
};