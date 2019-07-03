#!/bin/bash

apt install -y libmysqlclient-dev libcurl4-openssl-dev curl unixodbc-dev libjsoncpp-dev uuid-dev

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

dpkg -i --force-overwrite mycpp-1.0.0-Linux.deb
dpkg -i --force-overwrite dbsync-1.0.0-Linux.deb

ldconfig
