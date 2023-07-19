#include "server/sdk.h"
//
#include <thread>
#include <memory>
#include "model/application.h"
#include "model/json_loader.h"
#include "request_handler.h"
#include "command_parse.h"
#include "server/logger.h"
#include "database/postgres.h"
#include "database/db_functions.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;
namespace json = boost::json;
namespace fs = std::filesystem;

namespace {

// Запускает функцию fn на workers_count потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned workers_count, const Fn& fn) {
    workers_count = std::max(1u, workers_count);
    std::vector<std::jthread> workers;
    workers.reserve(workers_count - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--workers_count) {
        workers.emplace_back(fn);
    }
    fn();
}
}  // namespace


int main(int argc, const char* argv[]) {
    // инициализация логгера
    logger::InitBoostLogs();

    try {
        if (auto args = command_line::ParseCommandLine(argc, argv)){
            //  Инициализируем io_context
            const unsigned num_threads = std::thread::hardware_concurrency();
            net::io_context ioc(num_threads);

            // strand для выполнения запросов к API
            auto api_strand = net::make_strand(ioc);

            //Начинаем работать с базой данных
            postgres::Database db{pqxx::connection{db::GetConfigFromEnv().db_url}};
            db.CreateTable();

            //  Загружаем карту из файла и построить модель игры
            model::Game game = json_loader::LoadGame(args.value());
            auto app = std::make_shared<model::Application>(game, api_strand, args.value());

            app->RestoreGame();
            app->StartTicker();

            //  Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
                if (!ec) {
                    app->ForceSave();
                    ioc.stop();
                }
            });

            //  Создаём обработчик запросов в куче, управляемый shared_ptr
            auto handler = std::make_shared<http_handler::RequestHandler>(app, api_strand, args.value());

            //  Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
            const auto address = net::ip::make_address("0.0.0.0");
            constexpr net::ip::port_type port = 8080;

            http_server::ServeHttp(ioc, {address, port}, [&handler](auto&& req, auto&& send) {
                (*handler)(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
            });

            json::value custom_data {
                    {"port", port},
                    {"address", address.to_string()}
            };

            // Запись о том, что сервер запущен и готов обрабатывать запросы
            logger::Info("server started", custom_data);

            // Запускаем обработку асинхронных операций
            RunWorkers(std::max(1u, num_threads), [&ioc] {
                ioc.run();
            });
        }
    } catch (const std::exception& ex) {
        json::value custom_data {
                {"code", EXIT_FAILURE},
                {"exception", ex.what()}
        };

        logger::Fatal("server exited", custom_data);
        return EXIT_FAILURE;
    }

    logger::Info("server exited", {{"code", EXIT_SUCCESS }});
}
