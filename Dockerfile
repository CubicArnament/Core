# Используем базовый образ Ubuntu 22.04
FROM ubuntu:22.04

# Устанавливаем переменные окружения для неинтерактивной установки пакетов
ENV DEBIAN_FRONTEND=noninteractive

# Обновляем список пакетов и устанавливаем необходимые зависимости для сборки
# qemu-system-x86 включает эмуляторы для различных x86 архитектур, включая i386
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    make \
    nasm \
    gcc \
    gcc-multilib \
    g++-multilib \
    qemu-system-x86 \
    && apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Устанавливаем рабочую директорию внутри контейнера
WORKDIR /usr/src/myos

# Копируем исходный код проекта в рабочую директорию контейнера.
# Это позволяет собрать код, который был актуален на момент сборки образа.
# Однако для активной разработки чаще используется монтирование тома (см. инструкции по запуску).
COPY . .

# Команда по умолчанию не задана.
# Сборка будет запускаться командой 'make' при запуске контейнера, например:
# docker run --rm -v "$(pwd)":/usr/src/myos myos-builder make
# CMD ["make"]
