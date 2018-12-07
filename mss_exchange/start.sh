#!/bin/bash
#create by zyao 20181116 for 为了更方便的启动该项目而增加启动的启动脚本
#编译之前也要先执行该文件，因为要设置下环境变量，此处是临时设置，如果想方便可以将静态库路径加入环境变量LIBRARY_PATH修改主目录.bash_profile或.profile
echo "编译之前先执行该文件"
CURRENT_DIR=`pwd`
#指定程序静态链接库文件搜索路径
export LIBRARY_PATH=$LIBRARY_PATH:$CURRENT_DIR/lib:.

#启动交易所撮合主进程
cd bin
./mssmatch

