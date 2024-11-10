# QTStudy
This project is used for practicing the use of the QT framework. The code is allowed to be freely disseminated and used. Comments and messages are welcomed.
# Environment Dependencies
QT >= QT5.3.1

libuv
# Build
for build src/net/udp code,you need install libuv.In linux，excute 

`sudo apt-get install libuv1-dev`

in terminal.

In windows,you can download libuv source code to bulid from github.
# Project Introduction
|project|path|function|remark|
| --- | --- | --- | --- |
|QUdpLibuvOnlyRead|src/network/UDP|Based on the QT framework, call libuv in a child thread to efficiently receive UDP data.|only for read，can not send，see[https://zhuanlan.zhihu.com/p/5998483091](https://zhuanlan.zhihu.com/p/5998483091)|
|QUdpSocketTest SendMuchData|src/network/UDP|for compare with QUdpLibuvOnlyRead|QUdpSocketTest program recv data，SendMuchData program send data to QUdpSocketTest and QUdpLibuvOnlyRead|
# Excute
after build code，all excutable files will create in /bin dir.
