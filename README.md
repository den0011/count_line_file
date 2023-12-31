# count_line_file
Quick count of the number of lines in a file 

(Быстрый подсчет количества строк в файле)

Алгоритм подсчета строк, работает следующим образом:

1. Открывается файл для чтения с помощью `QFile`. Если файл не может быть открыт, функция возвращает -1, указывая на ошибку.
2. Затем определяется количество доступных потоков в системе с помощью `QThreadPool::globalInstance()->maxThreadCount()`. Это число определяет, на сколько частей будет разделен файл для параллельной обработки.
3. Файл разделяется на несколько частей (чанков), и для каждой части запускается отдельный поток с использованием `QtConcurrent::run`. Каждый поток отвечает за обработку своего чанка.
4. Внутри каждого потока происходит чтение данных из файла с помощью `QTextStream`. Поток обрабатывает только свою часть файла, прочитывая строки и подсчитывая количество строк в своем фрагменте.
5. Для того, чтобы учесть первую и последнюю строку в каждом чанке, используются функции `findLineStart` и `findLineEnd`. Эти функции корректируют начало и конец каждой части, чтобы учесть возможные недочеты в подсчете строк при параллельной обработке.
6. После завершения обработки всех частей, результаты суммируются, и получается общее количество строк в файле.
7. Затем все объекты `QFutureWatcher` удаляются, чтобы избежать утечек памяти.
8. Файл закрывается, и функция возвращает итоговое количество строк.

Таким образом, данный алгоритм разделяет файл на части и параллельно обрабатывает каждую часть в отдельном потоке, что позволяет ускорить процесс подсчета строк в больших файлах. При этом применяется коррекция для учета первой и последней строки в каждой части и достижения более точного результата.

