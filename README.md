# mssexchange_cpp
cpp写的一个中心化交易所撮合系统，基于redis内存库设计
金巴特交易所未来启用论坛地址：www.kingbut.com

目前，由于资源问题，只能挂靠在域名www.kingstargold.cn名下。

/*

2018-12-07:

金巴特交易所论坛地址： http://kingstargold.cn:8080/goldblog/c/%E9%87%91%E5%B7%B4%E7%89%B9%E4%B8%AD%E5%BF%83%E5%8C%96%E4%BA%A4%E6%98%93%E6%89%80


金巴特交易所qq交流群号： 959372362

金巴特交易所作者邮箱： myzhaoyao@126.com

*/

金巴特中心化交易所，核心模块mssmatch是mss撮合引擎，目前只支持单进程单合约撮合。未来将会扩展多合约的撮合。

一、编译

    本项目支持跨平台编译。

win7系统64位：vs2013编译   

centos7系统64位：GCC4.8.5编译

二、语言

    该项目仅使用了C/C++语言。

三、依赖库
