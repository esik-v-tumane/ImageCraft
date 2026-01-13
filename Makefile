# Компилятор и флаги
CC = gcc
CFLAGS = -std=c99 -Iinclude -Wall -Wextra -lm
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g -O0

# Директории
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR = int
ASSETS_DIR = assets
AUX_DIR = aux

# Исходные файлы (ищем все .c файлы в src/)
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Объектные файлы (заменяем src/ на int/ и .c на .o)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Имя исполняемого файла
TARGET = imagecraft

# Аргументы для запуска программы (можно переопределить при вызове)
RUN_ARGS ?= $(wildcard $(ASSETS_DIR)/*.bmp)

# Проверяем наличие утилит
BEAR := $(shell command -v bear 2> /dev/null)
CLANG_FORMAT := $(shell command -v clang-format 2> /dev/null)

# Основная цель по умолчанию (release сборка)
all:compile_commands.json auto-format build-release

# Release сборка
build-release: CFLAGS += $(RELEASE_FLAGS)
build-release: $(TARGET)

# Debug сборка
build-debug: CFLAGS += $(DEBUG_FLAGS)
build-debug: $(TARGET)

# Сборка и запуск в release режиме
run: auto-format prepare build-release
	@echo "Запуск программы в release режиме..."
	@echo "Аргументы: $(RUN_ARGS)"
	@echo ""
	./$(TARGET) $(RUN_ARGS)

# Сборка и запуск в debug режиме
run-debug: auto-format prepare build-debug
	@echo "Запуск программы в debug режиме..."
	@echo "Аргументы: $(RUN_ARGS)"
	@echo ""
	./$(TARGET) $(RUN_ARGS)

# Автоматическое форматирование (только если clang-format установлен)
auto-format:
ifdef CLANG_FORMAT
	@echo "Проверка и форматирование кода..."
	@$(CLANG_FORMAT) --dry-run --Werror -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) >/dev/null 2>&1 && \
		(echo "Код отформатирован верно.") || \
		($(CLANG_FORMAT) -i -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) && echo "Код отформатирован.")
else
	@echo "Предупреждение: clang-format не найден. Пропускаем форматирование."
endif

# Ручное форматирование (явный вызов)
format:
ifdef CLANG_FORMAT
	@echo "Форматирование кода..."
	@$(CLANG_FORMAT) -i -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h)
	@echo "Готово."
else
	@echo "Ошибка: clang-format не найден. Установите:"
	@echo "  Ubuntu/Debian: sudo apt install clang-format"
	@echo "  macOS: brew install clang-format"
	@exit 1
endif

# Проверка форматирования (без изменений)
check-format:
ifdef CLANG_FORMAT
	@echo "Проверка форматирования..."
	@$(CLANG_FORMAT) --dry-run --Werror -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) && \
		echo "✓ Форматирование корректно."
else
	@echo "Предупреждение: clang-format не найден. Пропускаем проверку."
endif

# Подготовка: копирование вспомогательных файлов
prepare:
	@mkdir -p $(OBJ_DIR)
	@if [ -f $(AUX_DIR)/help.txt ]; then \
		mkdir -p $(OBJ_DIR)/aux; \
		cp $(AUX_DIR)/help.txt $(OBJ_DIR)/aux/; \
		echo "Help file copied to build directory"; \
	fi

# Создание исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Компиляция исходных файлов в объектные
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание директории для объектных файлов
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Генерация compile_commands.json для LSP (clangd)
compile_commands.json: clean
ifdef BEAR
	@echo "Генерация compile_commands.json с помощью bear..."
	@$(BEAR) -- $(MAKE) $(TARGET)
	@echo "Файл compile_commands.json создан."
else
	@echo "Предупреждение: утилита 'bear' не найдена."
	@echo "Запускаю обычную сборку. compile_commands.json не будет создан."
	@$(MAKE) $(TARGET)
endif

# Псевдоним для удобства
bear-build: compile_commands.json

# Генерация compile_flags.txt для clangd (альтернатива)
compile_flags.txt:
	@echo "# Auto-generated flags for clangd" > $@
	@echo "$(CFLAGS)" | tr ' ' '\n' >> $@
	@echo "Файл compile_flags.txt обновлен."

# Очистка объектных файлов
clean-int:
	rm -rf $(OBJ_DIR)

# Очистка LSP-файлов
clean-lsp:
	rm -f compile_commands.json compile_flags.txt

# Очистка билда
clean-build: $(TARGET) clean-int

# Очистка вывода команды
clean-output:
	rm -rf output.bmp

# Очистка тестового запуска программы
clean-test:
	rm -rf test

# Полная очистка проекта
clean: clean-int clean-lsp clean-output clean-test
	rm -f $(TARGET)

# Вывод справки
help:
	@echo "Доступные команды:"
	@echo ""
	@echo "  make или  make build-release - сборка с автоформатированием"
	@echo "  make build-debug             - отладочная сборка с автоформатированием"
	@echo "  make run                     - сборка и запуск в release режиме"
	@echo "  make run-debug               - сборка и запуск в debug режиме"
	@echo "  make format                  - принудительное форматирование всего кода"
	@echo "  make check-format            - проверка форматирования без изменений"
	@echo "  make compile_commands.json   - генерация БД компиляции для LSP"
	@echo "  make bear-build              - то же, что и выше (псевдоним)"
	@echo "  make compile_flags.txt       - генерация файла флагов для LSP"
	@echo "  make clean-int               - удаление папки int/"
	@echo "  make clean-lsp               - удаление LSP-файлов"
	@echo "  make clean-build             - создание ./imagecraft и удаление int/"
	@echo "  make clean-test              - удаление папки test/"
	@echo "  make clean                   - полная очистка проекта"
	@echo "  make help                    - вывод этой справки"
	@echo ""
	@echo "Переменные:"
	@echo "  RUN_ARGS - аргументы для запуска программы"
	@echo "             Пример: make run RUN_ARGS=\"assets/image1.bmp assets/image2.bmp\""
	@echo ""
	@echo "Для работы LSP (clangd) рекомендуется:"
	@echo "  1. Установить bear (sudo apt install bear или brew install bear)"
	@echo "  2. Выполнить: make compile_commands.json"
	@echo "  3. Перезапустить редактор/IDE"
	@echo ""
	@echo "Особенности:"
	@echo "  • Перед сборкой код автоматически форматируется (если установлен clang-format)"
	@echo "  • Форматируется только при необходимости (безопасно для git)"
	@echo "  • Если clang-format не найден, сборка продолжается без форматирования"

# Файлы, которые не являются реальными целями
.PHONY: all build-release build-debug run run-debug \
        auto-format format check-format prepare \
        compile_commands.json bear-build compile_flags.txt \
        clean-int clean-lsp clean-build clean-output clean \
		help
