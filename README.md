#在NS2.34中加载GPSR协议
环境：
     ubuntu16.04
     ns2.34
[我的GPSR源码git库链接](https://github.com/Mangoleo/GPSR-source-code)

---
#####第一步
在ns-2.34的安装目录下，进入./ns-allinone-2.34/ns-2.34新建gpsr文件夹，并将gpsr源码文件复制进该文件夹。

---
#####第二步
在/ns-2.34/common下找到packet.h文件,修改：
`typedef unsigned int packet_t;后面`，`static packet_t PT_NTYPE=Y`前面
 添加 `static const packet_t PT_GPSR=X;`
此处的X是一个常数，按前面的顺序+1即可，要保证Y是大于X且为最后一项。
***
再在`class p_info·{}`内，`name_[PT_NTYPE]= "undefined";`前
添加`name_[PT_GPSR]=“gpsr”`

---
#####第三步
在/ns-2.34/trace下找到cmu-trace.cc文件，修改：
在开始的引用库语句中添加
`#include <gpsr/gpsr_packet.h>`
***
再在`void CMUTrace::format(Packet* p, const char *why)`内
添加`case PT_GPSR:
	break;`

---
#####第四步
在/ns-2.34/queue下，找到priqueue.cc文件，修改：
在`void
PriQueue::recv(Packet *p, Handler *h)`内
添加`case PT_GPSR`

---
#####第五步
在/ns-2.34/tcl/lib下，找到ns-packet.tcl，修改：
在`foreach prot {}`内
添加`gpsr`

---
#####第六步
在/ns-2.34/下，找到Makefile文件，修改：
在INCLUDES变量下，添加协议文件所在的路径（我是添加的绝对路径，别忘了换行符`\`）
***
在`OBJ_STL=`最后面加入
`gpsr/gpsr_neighbor.o \`
`gpsr/gpsr_sinklist.o \`
`gpsr/gpsr.o`

---
#####第七步
重新编译，在./ns-2.34下打开终端，执行以下命令：
`cd common`
`sudo touch packet.cc`
`cd ..`
`sudo make clean`这个指令可以清楚之前make产生的文件
`sudo make`
`sudo make install`
`sudo ldconfig`



