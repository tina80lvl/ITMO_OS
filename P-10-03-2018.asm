//
#!/bin/sh 
man sh //guide Shell
/Name //ищет клучевое слово по выводу man

//башизм - 

chmod +r test.sh //добавляет право на чтение
man sh //tutorial
cat //содержимое файла
fg
wc //сокращение слов (для подсчёта)
wc -l /ect/passwd
| //перенаправляет поток вывода в поток ввода (pipe)
cat a.log | sed -re "s/([^]+)/\1/" | \ sort | uniq -c | sort -rn | head -n 10 //example
awk
rm --recursive --force dir //GNU space
rm -r -f dir /** == **/ rm -rf dir 


//перенапривление ввода-вывода
wc -l /ect/passwd > ewc.stdout //создался файл и поток направился туда
wc -l < /ect/passwd // выведет количество строк
wc -l /ect/passwd > blabla //выводит количество строк passwd в blabla
> //rewrite
>> //add
$ (command) | wc -l //скобки создают отдельный процесс в shell
0 - stdin
1 - stdout
2 - sdterr
.test.sh 2> y //записал ошибки в файл у

grep //search



HW
//terminal
chmod +x ./test.sh
vim test.sh
./test.sh

//file
#!/bin/sh

set -e
set -u
set -x//debud output

o test

epoll_wait()
fcntl
setsockopt
accept4
DWARF






















