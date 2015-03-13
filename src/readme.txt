编译
不编译mysql

直接 make && make install 即可

带mysql编译

make CFLAG="-DHAVE_MYSQL_H -I/mysql头文件路径"
