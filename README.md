# Интерпретатор ITMOScript

Интерпретатор языка ITMOScript - это программа, которая позволяет интерпретировать файл с исходным кодом на языке ITMOScript, так как это например делает Python. Интерактивный режим (REPL) не обязателен, однако может быть реализован при желании.

Файлы с исходным кодом на ITMOScript имеют расширешие `is`.

Для удобства тестирования, Вам также требуется реализовать функцию принимающую на вход и выход поток, где входной поток - исходный код на ITMOScript, выходной - вызов функций вывода.


## Синтаксис

### Основные правила

- Код регистрозависим
- Пробелы и табуляции могут быть вставлены в любом количестве и не влияют на выполнение программы (в отличии от python)
- Для комментария используется `//`. Комментарии действует до конца строки
- Выражение не переносится на другую строку


### Стандартные типы данных

1. **Числа**
  - Знаковые
  - Двойная точность (соответствует double в языке C++)
  - Специальные литералы для булевой логики: `true` (1) и `false` (0)
  - Поддержка  нотации (например, `1.23e-4`)

2. **Строки**
  - Строковые литералы - всё, что окружено двойными кавычками `""`
  - Escape sequences должны корректно обрабатываться
  - "Some "string" type" не должно интерпретироваться, правильный синтаксис "Some \\"string\\" type"

3. **Списки**
  - Динамические массивы
  - Литералы в квадратных скобках: `[1, 2, 3]`
  - Индексация с нуля
  - Поддержка срезов (slices)

4. [**Функции**](#Функции)

5. **NullType**
  - Специальный тип означающий ничего
  - Специальный литерал этого типа `nil`

### Операторы

1. **Арифметические**
  - `+`, `-`, `*`, `/`, `%` (остаток от деления)
  - `^` (возведение в степень)
  - Унарные `+` и `-`

2. **Сравнения**
  - `==`, `!=`, `<`, `>`, `<=`, `>=`

3. **Логические**
  - `and`, `or`, `not`

4. **Присваивания**
  - `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `^=`

5. **Другие**
  - `[]` (индексация)


### Операторы и Стандартные типы данных


1. Числа (применяются в общепринятом понимании)
   - Арифметические
   - Сравнения
   - Логические
   - Присваивания


2. Строки
   - Сравнения (в лексикографическом порядке)
   - Арифметические
       - `+` - конкатенация двух строк
       - `-` - вычитает из строки суффикс (если первый аргумент оканчивается на второй)
       - `*` - применимо к строке и числу, результат - повторяет строку необходимое число раз (не обязательно целое)
   - Оператор `[]`
       - `s[n]` - возвращает n-й символ строки, отрицательные индексы отсчитываются с конца (как в питоне)
       - `s[:n]` - слайс, подстрока от первого до n символа (исключительно)
       - `s[n:]` - слайс, подстрока от n символа до конца
       - `s[m:n]` - слайс, подстрока от m-го символа до n-го(исключительно)
       - `s[:]` - слайс, копия строки
3. Списки
   - Арифметические
       - `+` - конкатенация
       - `*` - повторение (аналогично строке)
   - Оператор `[]`
       - Аналогично строке

4. NullType
   - Может бы сравним (`==`) с переменной любого типа. Возвращает `false` для всех случаем кроме `nil`
   - `!=` имеет обратный результат


### Условные операторы и циклы

1. **Условия**
  ```
  if условие then
      // выражения
  else if условие then
      // выражения
  else
      // выражения
  end if
  ```


2. **Циклы**


- `while`:
   ```
   while условие
       // выражения
   end while
   ```


- `for`:
   ```
   for i in последовательность
       // выражения
   end for
   ```

3. **Break & Continue**
 - Ключевые слова `break` и `continue` имеют привычный смысл при использование в циклах


### Функции


Функции в ITMOScript являются [объектом первого класса](https://en.wikipedia.org/wiki/First-class_function), и имеют отдельный тип данных. В отличии от С++, функции в ITMOScript объявляются через переменную. Для определения функции используется ключевое слово `function`. Количество аргументов не ограничивается. В качестве аргументов могут приниматься любые стандартные типы данных, включая сами функции.


```
имя = function(параметры)
   // выражения
   return результат
end function
```

Вызов функций осуществляется так же, как и в большинстве языков:

```
имя_функции(параметры_через_запятую)
```

Так как функции в ITMOScript являются обычным объектом, их можно [передавать как аргументы и возвращать](https://en.wikipedia.org/wiki/Higher-order_function). Синтаксис аналогичен другим типам данных (мы убрали `@`):

```
ilovefunctions("pass existing function...", func)

ilovefunctions("...or define one inplace", function(args)
    // do smth
end function)

alias = anotherfunc
```

Функции также могут определять другие функции внутри себя, однако внутренние функции не захватывают переменные из родительской функции. Другими словами, реализовывать [closures](https://en.wikipedia.org/wiki/Closure_(computer_programming)) не обязательно.


### Область видимости

Переменные и функции могут быть глобальными (объявлены в глобальной области видимости) и локальные (аргументы функций, локальные переменные)
Локальные переменные одной функции могут попасть в скоуп другой только через аргументы.
Затемнее внешних переменных может происходить только между глобальными переменными и аргументами функции.


## Стандартная библиотека

### Функции для работы с числами

- `abs(x)` - абсолютное значение
- `ceil(x)` - округление вверх
- `floor(x)` - округление вниз
- `round(x)` - округление до ближайшего целого
- `sqrt(x)` - квадратный корень
- `rnd(n)` - случайное целое от 0 до n-1
- `parse_num(s)`  - преобразует строку в число если возможно, иначе `nil`
- `to_string(n)` - преобразует число в строку


### Функции для работы со строками

- `len(s)` - длина строки
- `lower(s)` - в нижний регистр
- `upper(s)` - в верхний регистр
- `split(s, delim)` - разделение строки
- `join(list, delim)` - объединение списка в строку
- `replace(s, old, new)` - замена подстроки


### Функции для работы со списками

- `range(x, y, step)` - возвращает список чисел `[x; y)` с шагом `step`
- `len(list)` - длина списка
- `push(list, x)` - добавить элемент в конец
- `pop(list)` - удалить и вернуть последний элемент
- `insert(list, index, x)` - вставить элемент
- `remove(list, index)` - удалить элемент
- `sort(list)` - сортировка. Поведение при листе из разных типов -- implementation defined (но не UB!)


### Системные функции

- `print(x)` - вывод в поток вывода без дополнительных символов и перевода строки.
- `println(x)` - вывод в поток вывода с последующим переводом строки.
- `read()` - читает и возвращает строку из потока ввода
- `stacktrace()` - возвращает текущий стэк вызова функций. Формат стэка - на ваше усмотрение.

## Особенности реализации


1. **Динамическая типизация** - типы проверяются во время выполнения.
2. **Автоматическое управление памятью** - сборка мусора, переменные вышедшие из области видимости удаляются автоматически.
3. **Лексическая область видимости** - переменные видны в блоке, где объявлены, затемнение внешний имен так же как в С++.
4. **Интерпретация** - выполнение программы происходит построчно, ошибки синтаксиса проверяются в момент выполнения. При возникновении интерпретатор завершается с ошибкой.
5. **Safety** - выполнение некорректных операций не должно игнорироваться/вызывать ошибки на уровне вашего интерпретатора. Все ошибки ITMOScript должны быть обработаны и пойманы интерпретатором.
6. Простые типы (числа, nil) копируются по значению, сложные (строка, лист, функции) по ссылке. Другими словами, поведение при передаче аргументов и присвоении (`=`) аналогично Python.


Этот стандарт описывает базовую функциональность языка ITMOScript. Конкретные реализации могут добавлять дополнительные возможности.


## Тесты

Все вышеуказанный класс должен быть покрыты тестами, с помощью фреймворка [Google Test](http://google.github.io/googletest).
Тесты также являются частью задания, поэтому покрытие будет влиять на максимальный балл.


## Примеры

В директории `example` есть ряд примеров программ написанных на ITMOScript

  - fibonacci.is
  - maximum.is
  - fuzzBuzz.is

