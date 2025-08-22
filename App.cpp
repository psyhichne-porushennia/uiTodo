#include "App.h"
#include <iostream>

void to_json(nlohmann::json& j, const Task& task) {
    j = nlohmann::json{
        {"taskID", task.getTaskID()},
        {"text",   task.getText()},
        {"status", static_cast<int>(task.getStatus())}
    };
}

void from_json(const nlohmann::json& j, Task& task) {
    size_t id   = j.at("taskID").get<size_t>();
    std::string text = j.at("text").get<std::string>();
    Status status = static_cast<Status>(j.at("status").get<int>());
    task = Task(id, std::move(text), status);
}

bool FileRepository::saveTasks(const std::vector<Task>& tasks) {
    try {
        nlohmann::json j = tasks;
        std::ofstream file(filename, std::ios::trunc);
        if (!file.is_open()) return false;
        file << j.dump(4);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<Task> FileRepository::loadTasks() {
    std::vector<Task> tasks;
    std::ifstream file(filename);
    if (!file.is_open()) return tasks; // нема файлу — стартуємо порожньо

    try {
        nlohmann::json j;
        file >> j;
        tasks = j.get<std::vector<Task>>();
    } catch (...) {
        // битий JSON — ігноруємо, стартуємо порожньо
        tasks.clear();
    }
    return tasks;
}

std::vector<Task> TaskManager::getTasks() const {
    return tasks;
}

void TaskManager::addtask(std::string text) {
    // обрізаємо пробіли й пропускаємо пусті
    auto notSpace = [](unsigned char c){ return !std::isspace(c); };
    text.erase(text.begin(), std::find_if(text.begin(), text.end(), notSpace));
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.pop_back();
    if (text.empty()) return;
    tasks.emplace_back(tasks.size(), std::move(text), toDO);
}

void TaskManager::deleteTask(size_t taskID) {
    auto it = std::remove_if(tasks.begin(), tasks.end(),
                             [taskID](const Task& t){ return t.getTaskID() == taskID; });
    if (it != tasks.end()) {
        tasks.erase(it, tasks.end());
        reassignIds();
    }
}

void TaskManager::toggleDone(size_t taskID) {
    if (auto idx = indexOf(taskID)) {
        auto st = tasks[*idx].getStatus();
        setStatus(taskID, st == done ? toDO : done);
    }
}

void TaskManager::setStatus(size_t taskID, Status st) {
    if (auto idx = indexOf(taskID)) {
        tasks[*idx] = Task(taskID, tasks[*idx].getText(), st);
    }
}

void TaskManager::setText(size_t taskID, std::string_view text) {
    if (auto idx = indexOf(taskID)) {
        tasks[*idx] = Task(taskID, std::string{text}, tasks[*idx].getStatus());
    }
}

void TaskManager::saveTasks() {
    repository->saveTasks(tasks);
}

std::optional<size_t> TaskManager::indexOf(size_t taskID) const {
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [taskID](const Task& t){ return t.getTaskID() == taskID; });
    if (it == tasks.end()) return std::nullopt;
    return static_cast<size_t>(std::distance(tasks.begin(), it));
}

void TaskManager::reassignIds() {
    for (size_t i = 0; i < tasks.size(); ++i)
        tasks[i] = Task(i, tasks[i].getText(), tasks[i].getStatus());
}
