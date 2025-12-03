# Компилятор и флаги
CC = gcc
CFLAGS = -std=c99 -Iinclude -Wall -Wextra
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g -O0

# Директории
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = int
ASSETS_DIR = assets

# Исходные файлы (ищем все .c файлы в src/)
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Объектные файлы (заменяем src/ на int/ и .c на .o)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Имя исполняемого файла
TARGET = imagecraft

# Аргументы для запуска программы (можно переопределить при вызове)
RUN_ARGS ?= $(wildcard $(ASSETS_DIR)/*.bmp)

# Основная цель по умолчанию (release сборка)
all: build-release

# Release сборка
build-release: CFLAGS += $(RELEASE_FLAGS)
build-release: $(TARGET)

# Debug сборка
build-debug: CFLAGS += $(DEBUG_FLAGS)
build-debug: $(TARGET)

# Сборка и запуск в release режиме
run: build-release
	@echo "Запуск программы в release режиме..."
	@echo "Аргументы: $(RUN_ARGS)"
	./$(TARGET) $(RUN_ARGS)

# Сборка и запуск в debug режиме
run-debug: build-debug
	@echo "Запуск программы в debug режиме..."
	@echo "Аргументы: $(RUN_ARGS)"
	./$(TARGET) $(RUN_ARGS)

# Создание исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Компиляция исходных файлов в объектные
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание директории для объектных файлов
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Очистка объектных файлов
clean-int:
	rm -rf $(OBJ_DIR)

# Очистка билда
clean-build: $(TARGET) clean-int

# Полная очистка проекта
clean: clean-int
	rm -f $(TARGET)

# Вывод справки
help:
	@echo "Доступные команды:"
	@echo ""
	@echo "  make                    или  make build-release - сборка в режиме release"
	@echo "  make build-debug                              - сборка в режиме debug"
	@echo "  make run                                      - сборка и запуск в release режиме"
	@echo "  make run-debug                                - сборка и запуск в debug режиме"
	@echo "  make clean-int                                - удаление папки int/"
	@echo "  make clean-build                              - создание ./imagecraft и удаление int/"
	@echo "  make clean                                    - полная очистка проекта"
	@echo "  make help                                     - вывод этой справки"
	@echo ""
	@echo "Переменные:"
	@echo "  RUN_ARGS - аргументы для запуска программы (по умолчанию: все .bmp файлы из assets/)"
	@echo "             Пример: make run RUN_ARGS=\"assets/image1.bmp assets/image2.bmp\""
	@echo ""

# Файлы, которые не являются реальными целями
.PHONY: all build-release build-debug run run-debug clean-int clean-build clean help
