bin/ //лежат user-space утилиты (не требуют root доступа) 


//команды терминала
mount //число точек монтирования
cat comm
man 2 
creat //создаёт и возвращает файловый дискриптор
В качестве номера дискриптора используется (присваивается) наименьший неиспользуемый.
O_CREAT //flag 
getdents //strace -> dtruss
stat
find / -iname '*dir*' //иещт файлы

 //С code
open("/")-root-fd
openat(root-fd, var....) //защищено от //TOCTOU


HW
//read
man proc


strace поставить на комп 