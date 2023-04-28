# <center>IO多路复用</center>
文件描述符的特点：
1.非负整数
2.从最小可用的数字来分配
3.每个进程启动时默认打开0,1,2三个文件描述符
```c
int main (void)
{
	fd_set rset;
	int maxfd = -1;

	struct timeval tout;

	fd = socket ( ...);
	bind (fd, ...);
	listen (fd, ...);

	while (1) {
		maxfd = fd;
		FD_ZERO (&rset);

		FD_SET (fd, &rset);
		//依次把已经建立好连接fd加入到集合中,记录下来最大的文件描述符maxfd
		//...FIXME!!
#if 0
		select (maxfd + 1, &rset, NULL, NULL, NULL);
#else
		tout.tv_sec = 5;
		tout.tv_usec = 0;
		select (maxfd + 1, &rset, NULL, NULL, &tout);
#endif
		if (FD_ISSET (fd, &rset)) {
			newfd = accept (fd, ....);
		}
		//依次判断已建立连接的客户端是否有数据
		//...FIXME!

	}
	return 0;
}
```