# sqlquery
Qt sqlquery. 

Выполнение SQL запросов к БД SQLite, ODBC и пр. (зависит от наличия драйверов Qt). А так-же к произвольным текстовым файлам и cvs-файлам. Причем файлы можно загрузить пакетом. Файлы грузятся в SQLite БД в памяти. Каждый файл в таблицу со своим именем. Так что запросы можно формировать по нескольким файлам сразу.

Параметры командной строки.

sqlquery.exe [файл]

[файл]: 

- \*.txt - превращается в таблицу 1 типа: столбцы "row_id", "text", где "row_id" - номер строки с 1 до N, в "text" - содержимое строки.
- \*.txtfl | \*.txtdfl - файлы со списками путей к файлам, которые будет интерпретироваться как \*.txt и грузиться в таблицы соответственно их именам.
- \*.csv - создает таблицу с заголовками из строки с заголовком. А дальше добавляет в нее найденные данные.
- \*.csvfl | \*.csvdfl - файлы со списками путей к файлам, которые будет интерпретироваться как \*.csv и грузиться в таблицы соответственно их именам.

строки с префиксом "--" считаются комментарием и пропускаются.
Так-же пропускаются пустые строки.
в файлах \*.csvfl | \*.csvdfl можно указывать сепаратор. Пример: --separator=\'	\'

![screenshot](https://raw.githubusercontent.com/trdm/sqlquery/main/Doc/Screenshot_178.png)
