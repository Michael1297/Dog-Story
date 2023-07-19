#include "database/connection_pool.h"

namespace db {

ConnectionPool::ConnectionWrapper::ConnectionWrapper(std::shared_ptr<pqxx::connection>&& conn, PoolType& pool) noexcept
            : conn_{std::move(conn)}, pool_{&pool} {
}

pqxx::connection& ConnectionPool::ConnectionWrapper::operator*() const& noexcept {
    return *conn_;
}

pqxx::connection* ConnectionPool::ConnectionWrapper::operator->() const& noexcept {
    return conn_.get();
}

ConnectionPool::ConnectionWrapper::~ConnectionWrapper() {
    if (conn_) {
        pool_->ReturnConnection(std::move(conn_));
    }
}

ConnectionPool::ConnectionWrapper ConnectionPool::GetConnection() {
    std::unique_lock lock{mutex_};
    // Блокируем текущий поток и ждём, пока cond_var_ не получит уведомление и не освободится
        // хотя бы одно соединение
    cond_var_.wait(lock, [this] {
        return used_connections_ < pool_.size();
    });
    // После выхода из цикла ожидания мьютекс остаётся захваченным

    return {std::move(pool_[used_connections_++]), *this};
}

void ConnectionPool::ReturnConnection(ConnectionPtr&& conn) {
    // Возвращаем соединение обратно в пул
    {
        std::lock_guard lock{mutex_};
        assert(used_connections_ != 0);
        pool_[--used_connections_] = std::move(conn);
    }
    // Уведомляем один из ожидающих потоков об изменении состояния пула
    cond_var_.notify_one();
}

void ConnectionPoolSingleton::initSingleton(){
    instance = std::make_shared<ConnectionPoolSingleton>();
}

ConnectionPoolSingleton::ConnectionPoolSingleton() {
    auto db_url = GetConfigFromEnv();
    const unsigned num_threads = 1;
    pool = std::make_shared<ConnectionPool>(num_threads, [db_url] {
                                                auto conn = std::make_shared<pqxx::connection>(db_url);
                                                return conn;
                                            });
}

ConnectionPoolSingleton::ConnectionPoolSingletonPtr ConnectionPoolSingleton::GetInstance(){
    call_once(initInstanceFlag, ConnectionPoolSingleton::initSingleton);
    return instance;
}

ConnectionPoolSingleton::ConnectionPoolPtr ConnectionPoolSingleton::GetPool(){
    return pool;
}

std::string ConnectionPoolSingleton::GetConfigFromEnv(){
    if (const auto* url = std::getenv(db::DB_URL)) {
        return url;
    } else {
        throw std::runtime_error(db::DB_URL + (std::string)" environment variable not found");
    }
}

}