# cpp-messenger

Лёгкий клиент‑серверный мессенджер на C++ с поддержкой многопоточности и обменом приватными сообщениями в локальной сети через веб-сокеты.

## Структура
- `server/` – исходники и CMake для сервера  
- `client/` – исходники и CMake для клиента  

## Быстрый старт

```bash
# Клонируем
git clone https://github.com/CosmoCodeCraft/cpp-messenger.git
cd cpp-messenger

# Сборка сервера
mkdir build-server && cd build-server
cmake ../server && make
./messenger_server
cd ..

# Сборка клиента
mkdir build-client && cd build-client
cmake ../client && make
./messenger_client
