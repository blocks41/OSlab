#include <iostream>
#include <time.h>
#include <queue>
#include <string>
#include <list>
#include <vector>

using namespace std;
using Creation_tree=list

enum status_of_Process{ ready, running, blocked };   //���̵�״̬
enum Prior_of_Process{init, user, system};     //�������ȼ�

class Process {
private:
	PCB P_C_B;    //���̿��ƿ�

public:
	Process();     //����
	void Create();     //����
	void Destroy();    //����
	void Request();   //������Դ
	void Release();   //�ͷ���Դ
	void Time_out();    //ʱ���ж�
	void Control();    //����
};

class PCB {
private:
	string PID;   //��������
	status_of_Process status;    //���̵�״̬
	list
	Prior_of_Process Priority;   //�������ȼ�
		
public:
	friend class Process;
};

int main() {


}