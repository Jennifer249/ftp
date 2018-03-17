由于需要检测系统的用户名和密码，需要在root模式下开启服务器
编译：
gcc -lcrypt -o f -g fserver.c
运行：./f

客户端进入：
ftp 127.0.0.1 8000

实现的命令：login、pwd、bye、get、put、type、passive、system、mkdir、cd、ls
附加功能说明：
1、实现了ftp的主动模式、被动模式
2、可匿名登陆，而匿名登陆的用户要使用命令会被要求正常登陆
3、mkdir的权限管理，只能主目录下创建目录，不能向上级目录创建
4、登陆的用户和密码来自linux系统，通过/etc/shadow验证密码，因此只能在root模式下开启ftp服务器

