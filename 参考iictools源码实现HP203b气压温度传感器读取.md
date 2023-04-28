# <center>参考i2ctools源码实现HP203B气压温度传感器读取
由于开发的设备中要用到HP203B气压温度传感器，此设备要用到i2c协议进行数据的读取，我将以韦东山老师课程为指引，参考经典的i2c-tools源码，实现预期功能。
我的需求是：**运用HP203B传感器，读取温度及气压，并显示出来。**
## 1、 HP203B设备简介
### 1.1、功能简介
HP203B是一款超小型集高精度气压计、高度计和温度计于一体的传感器。内部集成了24位ADC，硅传感芯片，以及存放内部参数的OTP。该传感器通过设计公司获得的专利补偿算法在传感器器件片内进行采样，信号处理以及运算，最终计算出实际的直接结果值，所以外部应用MCU只需发出信号采集命令，待完成后，再通过I²C接口直接读取压力，温度及绝对海拔高度三者的实际值。
它的典型电路：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218184040128-1756335409.png)
长这样：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218183717845-1930617573.png)
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218184012054-700055764.png)

### 1.2、寄存器的操作
我们可以通过操作寄存器，设置我们检测各种参数的阈值，中断，并启用/禁用数据补偿；从0x00-0x0A是用来设置参数如补偿、阈值，0x0B-0x0D是用于中断的控制。
其寄存器列表如下图所示：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218175953082-488690050.png)
### 1.3、使用命令
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218175342222-1643267181.png)
发送不同的命令，我们就可以从其中读取相相应的数据，进行解析：
- SOFT_RST (0x06)
软复位命令。不管当时传感器的工作模式，一旦接收到此命令，传感器就立即自动复位，内部所有的寄存器将被重置为默认值后重新进入睡眠状态，等待接受主机来的命令。
- ADC_CVT (010, 3-bit OSR, 2-bit CHNL)
这个命令选择传感器内部的过采样率 OSR、传感信号输入通道 CHNL 及执行 ADC 的转换。前两位用于告诉传感器数据从哪个通道输出，接下来的三位用于设置采样率：

![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218181424658-550553151.png)
- READ_PT (0x10)
温度数据由 20 位 2 的补码格式组成，单位为摄氏度。温度的值由 24位 OUT_T_MSB,OUT_T_CSB,OUT_T_LSB 存储。最高 4 位的数据是无用，而最低有效 20 位代表温度的值。我们应当把这 20 位以 2 的补码的二进制值转换成一个整数，然后整数除以 100 获得最终结果。气压数据由 20 位 2 的补码格式组成，单位为帕。气压的值由 24 位 OUT_T_MSB,OUT_T_CSB OUT_T_LSB存储。最高 4 位的数据是无用，而最低有效 20 位代表气压的值。用户应当把这 20 位以 2 的补码的二进制值转换成一个整数，然后整数除以 100 获得最终结果。

![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218182116561-1275350799.png)
- READ_AT (0x11) 
温度数据由 20 位 2 的补码格式组成，单位为摄氏度。温度的值由 24 位 OUT_T_MSB,OUT_T_CSB 
OUT_T_LSB 存储。最高 4 位的数据是无用，而最低有效 20 位代表温度的值。用户应当把这 20 位以 2 的补码的二进制值转换成一个整数，然后整数除以 100 获得最终结果。
高度数据由 20 位 2 的补码格式组成，单位为米。高度的值由 24 位 OUT_T_MSB,OUT_T_CSB OUT_T_LSB存储。最高 4 位的数据是无用，而最低有效 20 位代表高度的值。用户应当把这 20 位以 2 的补码的二进制值转换成一个整数，然后整数除以 100 获得最终结果。

![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218182327092-424362179.png)
- READ_P (0x30) 
- READ_A (0x31)
- READ_T (0x32)
  这三种是单独读取气压，温度，海拔的数据的说明，数据格式和前文一样，感兴趣可以看数据手册。
- ANA_CAL (0x28)
  重新校准内部模块，一般用不到。
- READ_REG (0x80+6位寄存器地址)
  读取控制寄存器
- WRITE_REG (0xC0+6位寄存器地址)
  写入控制寄存器
### 1.4、传感器的I2C接口
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218183237415-765035264.png)
该传感器遵循I2C传输协议，当SCL拉低，SDA线上的信号开始变化，这是寄存器的地址和报文格式：
设备地址和CSB的引脚有关：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218184305175-1093219186.png)
eg：CSB借VDD，写状态：11101100 ：0xEC 
I2C协议：
- 单字节命令
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218192057638-1564429842.png)
- 写入寄存器
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218192114991-1668645455.png)
- 主机从设备读取寄存器的类型
第一帧是发送包含高 2 位二进制数 10 及后面跟着低 6 位的寄存器地址
的 READ_REG 命令。第一帧的格式与单字节的相同。在第二帧，该传感器将发送回寄存器中的数据当收到正确的设备地址及读位（R）之后。这种类型仅适用于使用 READ_REG 命令。
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218192140919-1478402471.png)
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218192155048-1494419601.png)
- 主机从设备读 3 字节或 6 字 ADC 数据
  ![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218192514885-2081309841.png)
我们连接到设备上看一下：使用的工具是I2C-tools，使用的开发板是RP-rv1126。
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218193527757-777659835.png)
此刻出现了一个问题，明明地址写的是0xEC，为什么在这表现的是0x76？博主比较疑惑，分析了一下，才发现，我们所写的0xEC是根据I2C格式的指令，我们看一下他含有什么？
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218193909214-1666674146.png)
实际上，第一个帧数据包含两个部分，一个是设备地址，一个是R/W,那就是说，我们发送的0xEC，也是包含这两个部分，11101100(0xEC) 由 111 0110(device address) 和 0(W)组成，实际的地址就是：0111 0110 (0x76)。
## 2. 结合i2ctools源码分析
写程序要比源码考虑的东西少很多，并且由于自身有限，有些地方还是比较模糊，所以摘抄关键的部分进行解析，然后提炼到自己的程序当中。
我们先看i2ctransfer.c
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230215155535862-2058986091.png)
第一个框是一些参数的选择，我们来看主要的部分：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218195017553-1776330693.png)
lookup_i2c_bus：解析命令，返回一个BUS的数值，我们本程序可以不用，直接输入i2cbus就行。
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218195630261-926632842.png)
接下来就是打开设备，linux系统中设备也是以文件的形式存在，我们需要用open打开设备，看是否正常。
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230215215545031-1357682777.png)
open_i2c_dev：传入了BUS的值，构造了设备名称，，并且打开了设备。
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218200120592-765254795.png)
接下来两个函数，他们本质上是完成了一个功能，第一个函数用于解析地址，第二个函数传入地址，我们看看set_slave_address干了什么事情：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218200449777-140302196.png)
set_slave_addr:我们前面打开了设备，但是设备中可能会挂载很多的地址，那么哪个才是我们需要的传感器的地址呢？我们就需要设置我们要操作哪个设备，下图可以看到，传入的参数中有force，什么是强制，一般来说，当设备显示UU，则证明已经有对应的驱动，不能直接访问，要通过驱动访问，如果我们要跳出驱动，坚持访问，就直接设置force为1即可。
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218200914415-2035219302.png)
设置完地址，我们就能找到设备了，接下来我们就该进行数据的传输了。
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218201226000-1639127871.png)
传输用到ioctl函数，控制这些函数我们都需要这个函数，没什么好说的，重点是他传输的数据，我们可以看到有一个重要的结构体：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218201613640-581698816.png)
这个结构体包含了什么，我们去linux源码看看，在\Linux-4.9.88\include\uapi\linux\i2c-dev.h中：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230218202228338-582054265.png)
我们从给ioctl函数中传入这样的结构体地址，也就是说，我们所有需要传输的信息就包含在内，我们可以看到里面有一个结构体，i2c_msg，我们再看看这个是怎么定义的：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230217144753482-779543311.png)
里面包含了addr,flags,len,buf还有一些宏定义，我们看看在整个i2ctransfer.c中，这个结构体是怎么设置的：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230215221759719-727311439.png)
初始化buf
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230215221857375-1598421268.png)
addr就是我们的设备的地址，buf里面就是需要写或者读的数据，flags用来表示传输方向，bit0等于0表示写,bit0等于I2C_M_RD表示读。
之后就是数据的发送了，可以看出，我们整体的流程就是：
打开**设备->设置地址->设置传输的信息->发送信息**即可，所以我们的主程序也这样来写：
```c
int main (int argc, char** argv) {
    /*打开设备*/
    open_i2c_dev(i2cbus, filename, sizeof(filename), 0)；
    /*设置设备地址*/
    set_slave_addr(file, _ADDRESS, 0);
    /*检查是否准备好，看寄存器的数值*/
    int HP203B_reg_read (int fd, unsigned char reg_addr, unsigned char * data_buf,int len);
    /*发送，写入命令，发送*/
    int HP203B_reg_write_cmd (int fd, unsigned char cmd);
    int HP203B_reg_read (int fd, unsigned char reg_addr, unsigned char * data_buf,int len);
}
```
这是大概的思路，具体的实现代码如下：
```c
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "i2cbusses.h"
/*./iic 3*/

#define _ADDRESS 0x76


int HP203B_reg_write_cmd (int fd, unsigned char cmd) {
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msgs;
    msgs.buf = NULL;
    msgs.addr = _ADDRESS;
    msgs.flags = 0;/*表示写*/
    msgs.len = 1;
    msgs.buf = &cmd;
    data.msgs = &msgs;
    data.nmsgs = 1;
    if (ioctl(fd, I2C_RDWR,&data) < 0) {
        printf("HP203B_reg_write_cmd err ! \n");
        return -1;
    }
    return 0;

}/*发送单字节命令*/
int HP203B_reg_read (int fd, unsigned char reg_addr, unsigned char * data_buf,int len) {
    int i ;
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msgs[2];
    for (i = 0; i < 2; i++) msgs[i].buf = NULL;
    msgs[0].addr = _ADDRESS;
    msgs[0].flags = 0;/*表示写*/
    msgs[0].len = 1;
    msgs[0].buf = &reg_addr;
    msgs[1].addr =  _ADDRESS;
    msgs[1].flags = I2C_M_RD;/*读*/
    msgs[1].len = len;
    msgs[1].buf = data_buf;
    data.msgs = msgs;
    data.nmsgs = 2;
    if (ioctl(fd, I2C_RDWR,&data) < 0) {
        printf("HP203B_reg_read err ! \n");
        return -1;
    }
    return 0;
}/*读取寄存器的值*/
int main(int argc, char** argv) {
    int i ;
    char filename[20];
    int file, i2cbus;
    unsigned char flag_ready;
    unsigned char rx_buf[10];
    float Temperature;
    unsigned int temp;
    unsigned int Pressure;
    if (argc != 2) {
        printf("usage is : ./iic [deviceNumber] \
        eg: ./iic 3 \n");
    }
    /*open device*/
    i2cbus = atoi(argv[1]);
    file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
    if (file < 0) {
        printf("can't open %s\n", filename);
        return -1;
    }
    /*set slave addr*/
    if(set_slave_addr(file, _ADDRESS, 0)) {
        printf("can't set_slave_addr\n");
		return -1;
    }
    while (1) {
    HP203B_reg_read(file, 0x80|0x0D, &flag_ready, 1);/*查看设备是否处于ready的状态*/
    sleep(1);
    if (flag_ready == 0x40) {
        HP203B_reg_write_cmd(file, 0x40| 2<<2);
        usleep(25);
        HP203B_reg_read(file, 0x10, rx_buf,6);
        temp = rx_buf[0] << 8 | rx_buf[1] | rx_buf[2];
        if(temp&0x800000)
        temp|=0xff000000;
        Pressure = rx_buf[3] << 8 | rx_buf[4] | rx_buf[5];
        Temperature = temp ;
        printf("Temperature : %fC       Pressure : %dPa    \n", Temperature, Pressure);
    }
    }
    close(file);
    return 0;
}
```
PS：里面有些函数我没有自己写，而是用的I2C-tools里面的函数，所以编译的时候要加函数的头文件及源码，列表如下：
![img](https://img2023.cnblogs.com/blog/3076422/202302/3076422-20230219193632771-292955685.png)
参考：[韦东山嵌入式课程](https://www.bilibili.com/video/BV1w4411B7a4?p=75&vd_source=efda0b84c0c4a82d1ebdef203c134d52)，HP203B数据手册