# Определение ОС
ifeq ($(OS),Windows_NT)
    # Сборка для OS Windows
    SYSTEM := Windows
    SEP := \\
    EXE_EXT := .exe
    MKDIR := mkdir
    RMDIR := rmdir /s /q
    RM := del /q
    NULL_OUT := >nul 2>&1
else
    # Сборка для OS Linux
    SYSTEM := Unix
    SEP := /
    EXE_EXT :=
    MKDIR := mkdir -p
    RMDIR := rm -rf
    RM := rm -f
    NULL_OUT := >/dev/null 2>&1
endif

# Параметры компиляции
CC := gcc
CFLAGS := -std=c99 -Iinclude -Wall -Wextra
LIBS := -lm
RELEASE_FLAGS := -O2
DEBUG_FLAGS := -g -O0

# Директории
SRC_DIR := src
OBJ_DIR := int
TARGET := imagecraft$(EXE_EXT)

# Исходные файлы и объекты
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Условие сохранения файлов
SAVE_INTERMEDIATE_FILES ?= no

# Цель по умолчанию
all: release

# Сборка в release режиме
release: CFLAGS += $(RELEASE_FLAGS)
release: clean-int $(TARGET)
ifeq ($(SAVE_INTERMEDIATE_FILES),no)
	@$(MAKE) clean-int
endif

# Сборка в debug режиме
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

# Создание исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $(TARGET)

# Компиляция объектов с использованием order-only prerequisite для директории
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание директории (команда зависит от ОС)
$(OBJ_DIR):
ifeq ($(SYSTEM),Windows)
	@if not exist $(OBJ_DIR) $(MKDIR) $(OBJ_DIR)
else
	@$(MKDIR) $(OBJ_DIR)
endif

# Удаление папки с объектами
clean-int:
ifeq ($(SYSTEM),Windows)
	@if exist $(OBJ_DIR) $(RMDIR) $(OBJ_DIR)
else
	@$(RMDIR) $(OBJ_DIR)
endif

# Полная очистка
clean: clean-int
ifeq ($(SYSTEM),Windows)
	@if exist $(TARGET) $(RM) $(TARGET)
else
	@$(RM) $(TARGET)
endif

# Справка в manpage стиле
help:
	@echo "NAME"
	@echo "    make - сборка проекта imagecraft"
	@echo ""
	@echo "SYNOPSIS"
	@echo "    make [target] [SAVE_INTERMEDIATE_FILES=yes]"
	@echo ""
	@echo "DESCRIPTION"
	@echo "    release (default)"
	@echo "        Сборка в release режиме. Директория $(OBJ_DIR) удаляется"
	@echo "        автоматически после завершения."
	@echo ""
	@echo "    debug"
	@echo "        Сборка для отладки (-g). Директория $(OBJ_DIR) сохраняется."
	@echo ""
	@echo "    clean"
	@echo "        Удаление исполняемого файла и всех временных объектов."
	@echo ""
	@echo "    clean-int"
	@echo "        Удаление только папки $(OBJ_DIR)."
	@echo ""
	@echo "OPTIONS"
	@echo "    SAVE_INTERMEDIATE_FILES=yes"
	@echo "        Сохранение папки $(OBJ_DIR) даже при release сборке."

.PHONY: all release debug clean clean-int help
