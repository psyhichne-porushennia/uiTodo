#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <optional>
#include <string_view>
#include <nlohmann/json.hpp>

enum Status { toDO, done, archived };

class Task {
public:
    Task() = default;
    Task(size_t id, std::string text, Status status = toDO)
        : text(std::move(text)), taskID(id), status(status) {}

    size_t getTaskID() const { return taskID; }
    const std::string& getText() const { return text; }
    Status getStatus() const { return status; }

    friend void to_json(nlohmann::json& j, const Task& task);
    friend void from_json(const nlohmann::json& j, Task& task);

private:
    std::string text;
    size_t taskID{};
    Status status{toDO};
};

void to_json(nlohmann::json& j, const Task& task);
void from_json(const nlohmann::json& j, Task& task);

class iTaskRepository {
public:
    virtual ~iTaskRepository() = default;
    virtual bool saveTasks(const std::vector<Task>& tasks) = 0;
    virtual std::vector<Task> loadTasks() = 0;
};

class FileRepository final : public iTaskRepository {
public:
    explicit FileRepository(std::string filename) : filename(std::move(filename)) {}
    bool saveTasks(const std::vector<Task>& tasks) override;
    std::vector<Task> loadTasks() override;

private:
    std::string filename;
};

class TaskManager {
public:
    explicit TaskManager(std::unique_ptr<iTaskRepository> repo)
        : repository{ std::move(repo) } {
        tasks = repository->loadTasks();
        for (size_t i = 0; i < tasks.size(); ++i)
            tasks[i] = Task(i, tasks[i].getText(), tasks[i].getStatus());
    }

    void addtask(std::string text);
    std::vector<Task> getTasks() const;
    void saveTasks();

    void deleteTask(size_t taskID);
    void toggleDone(size_t taskID);
    void setStatus(size_t taskID, Status st);
    void setText(size_t taskID, std::string_view text);

private:
    std::vector<Task> tasks;
    std::unique_ptr<iTaskRepository> repository;

    std::optional<size_t> indexOf(size_t taskID) const;
    void reassignIds();
};
