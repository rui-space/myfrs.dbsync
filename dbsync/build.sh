#!/bin/bash

apt-get install -y debconf-utils build-essential cmake dos2unix libjsoncpp-dev libmysqlclient-dev

tar xzvf odbc.driver/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit.tar.gz -C /usr/local
/usr/local/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/bin/myodbc-installer -d -a -n "MYSQL" -t "DRIVER=/usr/lib/libmyodbc8a.so;SETUP=/usr/lib/libmyodbc8a.so"

cp -rvf /usr/local/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/lib/libmyodbc8a.so /usr/lib
cp -rvf /usr/local/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/lib/libmyodbc8S.so /usr/lib
cp -rvf /usr/local/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/lib/libmyodbc8w.so /usr/lib
cp -rvf /usr/local/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/lib/libmyodbc8a.so /usr/lib64
cp -rvf /usr/local/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/lib/libmyodbc8S.so /usr/lib64
cp -rvf /usr/local/mysql-connector-odbc-8.0.11-linux-ubuntu16.04-x86-64bit/lib/libmyodbc8w.so /usr/lib64

cp -rvf odbc.driver/odbcinst.ini /etc/
cp -rvf odbc.driver/odbc.ini /etc/

mkdir -p build_ && cd build_
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make
make install
make package
