#!/bin/bash

# Сохраняем текущую директорию
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Переходим в корневую директорию проекта
cd "$PROJECT_ROOT"

echo "Текущая директория: $(pwd)"
echo "Ищем imagecraft..."

# Проверяем, существует ли исполняемый файл
if [ ! -f "./imagecraft" ]; then
    echo "Ошибка: imagecraft не найден в текущей директории!"
    echo "Текущая директория: $(pwd)"
    echo "Содержимое директории:"
    ls -la
    echo ""
    echo "Соберите проект: make"
    exit 1
fi

# Проверяем, существует ли исходное изображение
if [ ! -f "assets/lenna.bmp" ]; then
    echo "Ошибка: assets/lenna.bmp не найден!"
    echo "Поместите тестовое изображение в assets/lenna.bmp"
    exit 1
fi

echo "=== Тест 1: Обрезка 256x256 -> градации серого -> негатив ==="
./imagecraft assets/lenna.bmp test/test1.bmp -crop 256 256 -gs -neg
echo ""

echo "=== Тест 2: Повышение резкости -> выделение границ -> обрезка 400x300 ==="
./imagecraft assets/lenna.bmp test/test2.bmp -sharp -edge 0.2 -crop 400 300
echo ""

echo "=== Тест 3: Размытие -> медианный фильтр -> негатив ==="
./imagecraft assets/lenna.bmp test/test3.bmp -blur 1.5 -med 5 -neg
echo ""

echo "=== Тест 4: Обрезка 512x512 -> градации серого -> повышение резкости -> выделение границ ==="
./imagecraft assets/lenna.bmp test/test4.bmp -crop 512 512 -gs -sharp -edge 0.15
echo ""

echo "=== Тест 5: Медианный фильтр -> размытие -> негатив -> обрезка 600x400 ==="
./imagecraft assets/lenna.bmp test/test5.bmp -med 3 -blur 0.8 -neg -crop 600 400
echo ""

echo "=== Тест 6: ОГРОМНЫЕ значения для -crop (100000x100000) -> градации серого ==="
./imagecraft assets/lenna.bmp test/test6.bmp -crop 100000 100000 -gs
echo ""

echo "=== Все тесты завершены ==="
echo "Результаты сохранены в test/"
