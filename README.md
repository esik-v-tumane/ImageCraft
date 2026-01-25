# ImageCraft

## Аннотация
Учебный проект к лабораторной работе №1 по дисциплине "Исследовательский проект".

## Описание проекта
Проект посвящён разработке программы для применения фильтров к изображениям в формате BMP. Используется 24-битный BMP без сжатия и без таблицы цветов. Тип используемого DIB header — BITMAPINFOHEADER.

### Форматы работы программы

Формат команды для запуска программы:

`./imagecraft <input>.bmp <output>.bmp [-<filter1> <params1> [-<filter2> <params2> ...]]`

Вывод справки:

`./imagecraft -help`

`./imagecraft`

Вывод информации о версии:

`./imagecraft -version`

Вывод информации об изображении:

`./imagecraft -info <image>.bmp`

## Сборка
Проект держится на технологии Makefile и не требует внешних зависимостей

| Этап сборки | Windows | Linux |
|---|---|---|
| 1. Сборка проекта | MinGW: `mingw32-make`<br>Cygwin: `make` | `make` |
| 2. Запуск программы | `./imagecraft.exe` | `./imagecraft` |

Для ОС Windows при работе с MinGW следует использовать `mingw32-make`, иначе `make`. Для запуска программы использовать `./imagecraft.exe`.

## О программе

### Необязательные аргументы

Программа `imagecraft` принимает следующие дополнительные аргументы:

| Фильтр | Пояснение | Пример использования |
| :--- | :--- | :--- |
| `-help` | Показать справку. Также можно ничего не передавать программе | `./imagecraft -help` или `./imagecraft` |
| `-version` | Вывести версию | `./imagecraft -version` |
| `-info <image>.bmp` | Вывести информацию о bmp файле | `./imagecraft -info assets/lenna.bmp` |

### Реализованные фильтры

Ниже представлены реализованные фильтры и примеры их использования:

| Фильтр | Аргументы | Пример использования |
| :--- | :--- | :--- |
| `-crop` | `width height` | `./imagecraft assets/lenna.bmp output.bmp -crop 256 256` |
| `-gs` | - | `./imagecraft assets/lenna.bmp output.bmp -gs` |
| `-neg` | - | `./imagecraft assets/lenna.bmp output.bmp -neg` |
| `-sharp` | - | `./imagecraft assets/lenna.bmp output.bmp -sharp` |
| `-edge` | `threshold` | `./imagecraft assets/lenna.bmp output.bmp -edge 0.2` |
| `-blur` | `sigma` | `./imagecraft assets/lenna.bmp output.bmp -blur 1.5` |
| `-med` | `window` | `./imagecraft assets/lenna.bmp output.bmp -med 5` |
| `-crystal` | `x y radius` | `./imagecraft assets/lenna.bmp output.bmp -crystal 5 10 40` |
