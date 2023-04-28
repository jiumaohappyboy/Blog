#  关于文件IO缓冲区的思考
## 问题的提出：
关于我在实现socket的时候，有一个打印的问题一直在困扰我：
我的程序具体的实现功能是：写一个客户端和服务器，客户端写入数据，服务器打印出来，并返回给服务器，其中客户端中的输入是用select管理的
客户端发送数据的程序是这样的：
![img](https://img2023.cnblogs.com/blog/3076422/202303/3076422-20230310142950846-1840271772.png)
从键盘接受到东西以后写回套接字
服务器中接收数据的程序是这样的：
![img](https://img2023.cnblogs.com/blog/3076422/202303/3076422-20230310143109945-1183262559.png)
这里判断没有问题以后，将接收的信息打印出来
当程序运行起来以后，服务器端的输出有了问题：
客户端：![img](https://img2023.cnblogs.com/blog/3076422/202303/3076422-20230310143501992-539668900.png)
服务器端：
![img](https://img2023.cnblogs.com/blog/3076422/202303/3076422-20230310143537174-1582327239.png)
途中标红的部分，为什么我多输出了这个字符串？？？
## 解决
检查问题后，发现，我只要将read函数中的读取数据设置成大于等于write的数据，就避免这种问题，我对此很疑惑，于是写了测试程序去测试关于read和write：
```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <strings.h>


int main()
{
        int fd1 = open("./1.txt", O_RDWR);
        char readbuf[BUFSIZ];
        char writebuf[BUFSIZ] = {"this is a buff test"};
        int ret = write(fd1,writebuf,sizeof(writebuf));
        if (ret < 0) perror("write");
        int fd2 = open("./1.txt", O_RDWR);
        while (1) 
        {
        ret = read(fd2,readbuf,sizeof(readbuf) - 1);
        //ret = read(fd2,readbuf,sizeof(readbuf));
        if (ret < 0) perror("read");
        if (ret > 0){
        //printf("%d\n",sizeof(BUFSIZ));
        printf("%d ",ret);
        printf("%s\n",readbuf);
        sleep(1);
        bzero(readbuf,BUFSIZ);
        }
        }
        close(fd1);
        close(fd2);

}
```
我这里在writebuf中并没有加换行符，也就是说，read在文件中读取数据的时候，并不会因为换行符而清空接受区，而是一直在读取，我将read的数据大小设置小于write的大小,程序运行结果如下：
![img](https://img2023.cnblogs.com/blog/3076422/202303/3076422-20230310144606286-35554933.png)
程序并没有一直打印 我写的数据串，我们用gdb看看程序里面发生了什么：
![img](https://img2023.cnblogs.com/blog/3076422/202303/3076422-20230310144851124-1495877732.png)
![img](https://img2023.cnblogs.com/blog/3076422/202303/3076422-20230310145111092-402857036.png)
在这我第一次读取完了以后，并没有什么异常，但有趣的是，第二次读取的字符串里面出现了"\0",才读取了文件的结束符，后续我们再读取的时候，buff里面就储存了"\0"也就是说其实在我们的输出中，buff并不是没有读到数据，而是读到了，但是它的前面有个换行符，并没有输出出来，但是我们此时read确确实实是读到了数据，也就不存在read出现perror。
## 总结
经过以上分析，我们可以发现，read再读取了小于缓冲区的数据时，第二次读取的时候就会没有读取的数据读取进去。
我试着理解了一下内核缓冲机制：
**read&write，read把数据从内核缓冲区复制到进程缓冲区，write把数据从进程缓冲区复制到内核缓冲区，它们不等价于数据在内核缓冲区和磁盘之间的交换。**
那就是说我们write写入的时BUFSIZ的数据到内核的缓冲区，read的时候读取了BUFSIZ - 1的数据，还有数据留在缓冲区中，在第二次的时候，虽然读到了，但是因为有结束符的存在，并没有让我们达到理想的效果。
正确的解决方法就是，我们将写的数据的长度写为strlen()会很好的解决问题
ps：困扰的问题找到了答案，如果有什么不对的地方，欢迎批评指正。