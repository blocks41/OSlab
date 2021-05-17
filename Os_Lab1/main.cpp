#include <iostream>
#include <time.h>
#include <queue>
#include <string>
#include <list>
#include <vector>

using namespace std;
using Creation_tree=list

enum status_of_Process{ ready, running, blocked };   //进程的状态
enum Prior_of_Process{init, user, system};     //进程优先级

class Process {
private:
	PCB P_C_B;    //进程控制块

public:
	Process();     //创建
	void Create();     //创建
	void Destroy();    //撤销
	void Request();   //请求资源
	void Release();   //释放资源
	void Time_out();    //时钟中断
	void Control();    //调度
};

class PCB {
private:
	string PID;   //进程名称
	status_of_Process status;    //进程的状态
	list
	Prior_of_Process Priority;   //进程优先级
		
public:
	friend class Process;
};

int main() {


}