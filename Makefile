ifeq ($(findstring mingw,$(MAKE)),)
    $(warning Рекомендуется использовать mingw32-make на Windows)
endif

# Определение ОС
ifeq ($(OS),Windows_NT)
    # Windows
    SYSTEM := Windows
    MKDIR := mkdir
    RMDIR := rmdir /s /q
    RM := del /q
    CP := copy
    SEP := \\
    NULL_OUT := >nul 2>&1
    EXE_EXT := .exe
    # Проверяем доступность mingw32-make или make
    ifeq ($(MAKE),)
        MAKE := make
    endif
else
    # Unix-like системы (Linux, macOS)
    SYSTEM := Unix
    MKDIR := mkdir -p
    RMDIR := rm -rf
    RM := rm -f
    CP := cp
    SEP := /
    NULL_OUT := >/dev/null 2>&1
    EXE_EXT :=
    
    # Определяем macOS
    ifeq ($(shell uname),Darwin)
        SYSTEM := macOS
    endif
endif

# Убедимся, что пути используют правильный разделитель
fix_path = $(subst /,$(SEP),$1)

# Компилятор и флаги
CC = gcc
CFLAGS = -std=c99 -Iinclude -Wall -Wextra -lm
RELEASE_FLAGS = -O2
DEBUG_FLAGS = -g -O0

# Директории (используем правильные разделители)
SRC_DIR = $(call fix_path,src)
INCLUDE_DIR = $(call fix_path,include)
OBJ_DIR = $(call fix_path,int)
ASSETS_DIR = $(call fix_path,assets)
AUX_DIR = $(call fix_path,aux)

# Исходные файлы (ищем все .c файлы в src/)
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Объектные файлы (заменяем src/ на int/ и .c на .o)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Имя исполняемого файла
TARGET = image_craft$(EXE_EXT)

# Аргументы для запуска программы (можно переопределить при вызове)
RUN_ARGS ?= $(wildcard $(ASSETS_DIR)/*.bmp)

# Проверяем наличие утилит кросс-платформенно
ifeq ($(SYSTEM),Windows)
    # На Windows ищем с помощью where
    BEAR := $(shell where bear 2>nul)
    CLANG_FORMAT := $(shell where clang-format 2>nul)
else
    # На Unix-like системах используем command -v
    BEAR := $(shell command -v bear 2>/dev/null)
    CLANG_FORMAT := $(shell command -v clang-format 2>/dev/null)
endif

# Основная цель по умолчанию (release сборка)
all: auto-format build-release

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
	@echo "Приступаю к сборке"


shit:
ifdef CLANG_FORMAT
	@echo "Проверка и форматирование кода..."
ifeq ($(SYSTEM),Windows)
	@$(CLANG_FORMAT) --dry-run --Werror -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) >nul 2>&1 && ( \
		echo "Код отформатирован верно.") || ( \
		$(CLANG_FORMAT) -i -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) && echo "Код отформатирован.")
else
	@$(CLANG_FORMAT) --dry-run --Werror -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) $(NULL_OUT) && \
		(echo "Код отформатирован верно.") || \
		($(CLANG_FORMAT) -i -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) && echo "Код отформатирован.")
endif
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
ifeq ($(SYSTEM),Windows)
	@echo "  Windows: choco install llvm или скачайте с https://llvm.org/"
else ifeq ($(SYSTEM),macOS)
	@echo "  macOS: brew install clang-format"
else
	@echo "  Ubuntu/Debian: sudo apt install clang-format"
	@echo "  Fedora: sudo dnf install clang-tools-extra"
endif
	@exit 1
endif

# Проверка форматирования (без изменений)
check-format:
ifdef CLANG_FORMAT
	@echo "Проверка форматирования..."
ifeq ($(SYSTEM),Windows)
	@$(CLANG_FORMAT) --dry-run --Werror -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) >nul 2>&1 && \
		echo "✓ Форматирование корректно."
else
	@$(CLANG_FORMAT) --dry-run --Werror -style=file $(SRCS) $(wildcard $(INCLUDE_DIR)/*.h) && \
		echo "✓ Форматирование корректно."
endif
else
	@echo "Предупреждение: clang-format не найден. Пропускаем проверку."
endif

# Подготовка: копирование вспомогательных файлов
prepare:
	@$(MKDIR) $(OBJ_DIR)
	@if exist $(AUX_DIR)\help.txt ( \
		$(MKDIR) $(OBJ_DIR)$(SEP)aux && \
		$(CP) $(AUX_DIR)$(SEP)help.txt $(OBJ_DIR)$(SEP)aux$(SEP) && \
		echo "Help file copied to build directory" \
	) else ifneq ($(wildcard $(AUX_DIR)/help.txt),) ( \
		$(MKDIR) $(OBJ_DIR)$(SEP)aux && \
		$(CP) $(AUX_DIR)/help.txt $(OBJ_DIR)$(SEP)aux/ && \
		echo "Help file copied to build directory" \
	)

# Создание исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Компиляция исходных файлов в объектные
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание директории для объектных файлов
$(OBJ_DIR):
	$(MKDIR) $(OBJ_DIR)

# Генерация compile_commands.json для LSP (clangd)
compile_commands.json: clean
ifdef BEAR
	@echo "Генерация compile_commands.json с помощью bear..."
	@bear -- $(MAKE) $(TARGET)
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
ifeq ($(SYSTEM),Windows)
	@if exist $(OBJ_DIR) $(RMDIR) $(OBJ_DIR)
else
	$(RMDIR) $(OBJ_DIR)
endif

# Очистка LSP-файлов
clean-lsp:
ifeq ($(SYSTEM),Windows)
	@if exist compile_commands.json $(RM) compile_commands.json
	@if exist compile_flags.txt $(RM) compile_flags.txt
else
	$(RM) compile_commands.json compile_flags.txt
endif

# Очистка билда
clean-build: $(TARGET) clean-int

# Очистка вывода команды
clean-output:
ifeq ($(SYSTEM),Windows)
	@if exist output.bmp $(RM) output.bmp
else
	$(RM) output.bmp
endif

# Очистка тестового запуска программы
clean-test:
ifeq ($(SYSTEM),Windows)
	@if exist test $(RMDIR) test
else
	$(RMDIR) test
endif

# Полная очистка проекта
clean: clean-int clean-lsp clean-output clean-test
ifeq ($(SYSTEM),Windows)
	@if exist $(TARGET) $(RM) $(TARGET)
else
	$(RM) $(TARGET)
endif

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
ifeq ($(SYSTEM),Windows)
	@echo "  1. Установить bear через MSYS2: pacman -S bear"
	@echo "  2. Выполнить: make compile_commands.json"
	@echo "  3. Перезапустить редактор/IDE"
else
	@echo "  1. Установить bear (sudo apt install bear или brew install bear)"
	@echo "  2. Выполнить: make compile_commands.json"
	@echo "  3. Перезапустить редактор/IDE"
endif
	@echo ""
	@echo "Особенности:"
	@echo "  • Перед сборкой код автоматически форматируется (если установлен clang-format)"
	@echo "  • Форматируется только при необходимости (безопасно для git)"
	@echo "  • Если clang-format не найден, сборка продолжается без форматирования"

# Файлы, которые не являются реальными целями
.PHONY: all build-release build-debug run run-debug \
        auto-format format check-format prepare \
        compile_commands.json bear-build compile_flags.txt \
        clean-int clean-lsp clean-build clean-output clean clean-test \
        help
