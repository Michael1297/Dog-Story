# Не просто создаём образ, но даём ему имя build
FROM gcc:11.3 as build

RUN apt update && \
    apt install -y \
      python3-pip \
      cmake \
    && \
    pip3 install conan==1.*

# Запуск conan
COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing -s build_type=Release -s compiler.libcxx=libstdc++11

# Папка data больше не нужна
COPY ./src /app/src
COPY ./include /app/include
COPY ./tests /app/tests
COPY CMakeLists.txt /app/

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build .

# Второй контейнер в том же докерфайле
FROM ubuntu:22.04 as run

# Создаем пользователя www
RUN groupadd -r www && useradd -r -g www www
USER www

# Скопируем приложение со сборочного контейнера в директорию /app.
# Не забываем также папку data, она пригодится.
COPY --from=build /app/build/src/game_server /app/
COPY ./data /app/data
COPY ./static /app/static

# Запускаем игровой сервер
# обычный пуск
ENTRYPOINT ["/app/game_server", "-c", "/app/data/config.json", "-w", "/app/static"]
# запуск с тиком
# ENTRYPOINT ["/app/game_server", "-c", "/app/data/config.json", "-w", "/app/static", "--tick-period", "500", "--randomize-spawn-points", "true", "--state-file", "save_file", "--save-state-period", "3500"]