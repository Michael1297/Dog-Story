#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <cassert>
#include <pqxx/connection>

namespace db {

//constexpr const char DB_URL[]    = "postgres://postgres:password@localhost:5432/retired_players";
constexpr const char DB_URL[]    = "GAME_DB_URL";

class ConnectionPool {
    using PoolType = ConnectionPool;
    using ConnectionPtr = std::shared_ptr<pqxx::connection>;

public:
    class ConnectionWrapper {
    public:
        ConnectionWrapper(std::shared_ptr<pqxx::connection>&& conn, PoolType& pool) noexcept;
        ConnectionWrapper(const ConnectionWrapper&) = delete;
        ConnectionWrapper& operator=(const ConnectionWrapper&) = delete;
        ConnectionWrapper(ConnectionWrapper&&) = default;
        ConnectionWrapper& operator=(ConnectionWrapper&&) = default;
        pqxx::connection& operator*() const& noexcept;
        pqxx::connection& operator*() const&& = delete;
        pqxx::connection* operator->() const& noexcept;
        ~ConnectionWrapper();

    private:
        std::shared_ptr<pqxx::connection> conn_;
        PoolType* pool_;
    };

    // ConnectionFactory is a functional object returning std::shared_ptr<pqxx::connection>
    template <typename ConnectionFactory>
    ConnectionPool(size_t capacity, ConnectionFactory&& connection_factory) {
        pool_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            pool_.emplace_back(connection_factory());
        }
    }

    ConnectionWrapper GetConnection();

private:
    void ReturnConnection(ConnectionPtr&& conn);

    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::vector<ConnectionPtr> pool_;
    size_t used_connections_ = 0;
};

class ConnectionPoolSingleton{
private:
    using ConnectionPoolSingletonPtr = std::shared_ptr<ConnectionPoolSingleton>;
    using ConnectionPoolPtr = std::shared_ptr<ConnectionPool>;
    inline static std::once_flag initInstanceFlag;
    inline static ConnectionPoolSingletonPtr instance;
    ConnectionPoolPtr pool;

    static void initSingleton();

public:
    ConnectionPoolSingleton();
    ~ConnectionPoolSingleton() = default;
    ConnectionPoolSingleton(const ConnectionPoolSingleton&) = delete;
    ConnectionPoolSingleton& operator=(const ConnectionPoolSingleton&) = delete;
    static ConnectionPoolSingletonPtr GetInstance();
    ConnectionPoolPtr GetPool();
    static std::string GetConfigFromEnv();
};

}  // namespace db