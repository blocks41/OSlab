#include <iostream>
#include <string>
#include <list>
#include <vector>

#define ERROR -1
#define init_ 0
#define user_ 1
#define system_ 2

using namespace std;
using Prior_Type = int;

enum status_Type{ ready, running, blocked };   //进程的状态

struct PCB_Type;
using PCB_Pointer = struct PCB_Type*;
class RCB_Type;
struct Resource_Type;
struct Creation_Tree_Type;

//PCB结构
struct PCB_Type {
	string PID;   //进程名称
	vector<Resource_Type> Resource;   //占有的资源、资源数
	status_Type status;    //进程的状态
	Creation_Tree_Type* Creation_Tree;   //进程树结构
	Prior_Type Priority;   //进程优先级
};
//RCB结构
class RCB_Type {
private:
	int R[5] = { 0, 1, 2, 3, 4 };

public:
	friend class Test_Shell;

	int Get_RCB(int rid) {    //获取空余资源数
		return R[rid];
	}
	void plus_R(int rid, int n) {   //增加资源数
		R[rid] += n;
	}
	void subs_R(int rid, int n) {   //减少资源数
		R[rid] -= n;
	}
};

//资源结构
struct Resource_Type {
	int RID;   //资源名称
	int Count;   //占用资源数
};

struct Creation_Tree_Type {
	PCB_Pointer Parent;   //父进程
	list<PCB_Pointer> Children;   //子进程
};

//进程管理类Test_Shell
class Test_Shell {   
private:
	//三个优先级的就绪队列,这里不采用vector,因为进程要频繁的插入删除
	PCB_Pointer Running=nullptr;  //假设只能有一个运行
	list<PCB_Pointer> user_Ready_List;
	list<PCB_Pointer> system_Ready_List;
	list<PCB_Pointer> Block_List;  //阻塞队列
	RCB_Type RCB;   //资源控制块

public:
	//初始化环境
	void init() {
		//Init进程在启动时创建，可以用来创建第一个系统进程或者用户进程，即不管谁来都把它挤走。
		PCB_Pointer PCB = new PCB_Type;
		PCB->Creation_Tree = new Creation_Tree_Type;
		PCB->PID = "init";   
		PCB->Priority = init_;  //优先级为最低
		PCB->status = running;
		Running = PCB;   //直接放入运行
	}
	//创建进程
	void Create(string PID_, Prior_Type Priority_) { 
		//创建PCB
		PCB_Pointer PCB=new PCB_Type;
		PCB->Creation_Tree = new Creation_Tree_Type;
		//初始化PCB
		PCB->PID = PID_;
		PCB->Priority = Priority_;
		//连接父亲节点和兄弟节点
		PCB->Creation_Tree->Parent = Running;
		if (Running) Running->Creation_Tree->Children.push_back(PCB);
		//将PCB加到对应就绪队列中
		PCB->status = ready;
		if (PCB->Priority == user_) user_Ready_List.push_back(PCB);
		if (PCB->Priority == system_) system_Ready_List.push_back(PCB);
		//进程调度
		Scheduler();
	}
	//撤销进程
	void Destroy(string pid) {
		PCB_Pointer PCB=nullptr;
		//得到进程的PCB，所有队列里找
		if (Running->PID==pid) PCB = Running;
		for (auto begin = user_Ready_List.begin(); begin != user_Ready_List.end(); begin++) {
			if ((*begin)->PID==pid) {
				PCB = (*begin);
				break;
			}
		}
		for (auto begin = system_Ready_List.begin(); begin != system_Ready_List.end(); begin++) {
			if ((*begin)->PID == pid) {
				PCB = (*begin);
				break;
			}
		}
		for (auto begin = Block_List.begin(); begin != Block_List.end(); begin++) {
			if ((*begin)->PID == pid) {
				PCB = (*begin);
				break;
			}
		}
		//撤销该进程及其子孙进程
		if (PCB) {
			Kill_Tree(PCB);
			Scheduler();
		}
	}
	//请求资源
	void Request(int rid, int Num_of_Request) {   //请求资源
		int Num_of_Resource = RCB.Get_RCB(rid);   //获取资源剩余
		//在PCB中加入信息 
		Resource_Type R;
		R.RID = rid;
		R.Count = Num_of_Request;
		Running->Resource.push_back(R);   

		if (Num_of_Resource >= Num_of_Request) {   //剩余资源数大于请求的
			RCB.subs_R(rid, Num_of_Request);      //减去分配的资源
		}
		else
		{
			if (Num_of_Request > rid) {  //当申请的资源数大于资源的总数，就打印错误并退出
				cout << "申请资源数大于资源总数！";
				exit(ERROR);
			}
			Running->status = blocked;
			Block_List.push_back(Running);  //申请资源数大于剩余资源数，就加入阻塞队列
			//从运行队列中删除
			Running = nullptr;
			Scheduler();
		}
	}
	//释放资源
	void Release(int rid, int Num_of_Release) {
		//将资源r从当前进程占用的资源列表里移除，并且资源r的可用数量从u变为u+n
		for (auto begin = Running->Resource.begin(); begin != Running->Resource.end(); begin++) {
			if ((*begin).RID == rid) {
				RCB.plus_R(rid, Num_of_Release);
				Running->Resource.erase(begin);
				break;
			}
		}
		//基于优先级的抢占式调度策略，因此当有进程获得资源时，需要查看当前的优先级情况并进行调度
		Scheduler(); 
	}
	//时钟中断
	void Time_out() {
		Running->status = ready;
		if (Running->Priority == user_) {
			user_Ready_List.push_back(Running);
			Running = user_Ready_List.front();
			user_Ready_List.pop_front();
		}
		if (Running->Priority == system_) {
			system_Ready_List.push_back(Running);
			Running = system_Ready_List.front();
			system_Ready_List.pop_front();
		}
		Running->status = running;
	}
	//调度
	void Scheduler() {
		//如果阻塞队列不为空
		for (auto begin = Block_List.begin(); begin != Block_List.end();) {
			int flag = 1;   //用来标志是否满足资源分配
			//先检查
			for (int i = 0; i < (*begin)->Resource.size(); i++) {
				int rid = (*begin)->Resource[i].RID;//需求资源id
				int Num_of_request = (*begin)->Resource[i].Count;  //需求资源数
				//如果申请的资源数大于剩余，下一个循环
				if (RCB.R[rid] < Num_of_request) {
					flag = 0;
					break;
				}
			}
			//如果资源数满足，就唤醒这个队列
			if (flag) {
				//分配资源
				for (int i = 0; i < (*begin)->Resource.size(); i++) {
					int rid = (*begin)->Resource[i].RID;//需求资源id
					int Num_of_request = (*begin)->Resource[i].Count;  //需求资源数
					RCB.subs_R(rid, Num_of_request);
				}
				(*begin)->status = ready;
				// 插入到对应就绪队列
				if ((*begin)->Priority == user_) user_Ready_List.push_back((*begin));
				if ((*begin)->Priority == system_) system_Ready_List.push_back((*begin));
				Block_List.erase(begin); // 从资源r的阻塞队列中移除
				begin = Block_List.begin();
			}
			else
			{
				begin++;
			}
		}
		//当没有运行进程时
		if (!Running) {
			if (!system_Ready_List.empty()) {
				Running = system_Ready_List.front();
				system_Ready_List.pop_front();
			}
			else if (!user_Ready_List.empty()) {
				Running = user_Ready_List.front();
				user_Ready_List.pop_front();
			}
		}
		//当就绪队列头的进程优先级更高，就将正在运行的加入就绪队列尾
		else if (!system_Ready_List.empty() && system_Ready_List.front()->Priority > Running->Priority) {
			user_Ready_List.push_back(Running);
			Running = system_Ready_List.front();
			system_Ready_List.pop_front();
		}
		else if (!user_Ready_List.empty() && user_Ready_List.front()->Priority > Running->Priority) {
			Running = user_Ready_List.front();
			user_Ready_List.pop_front();
		}
	}
	//杀死进程，及删除队列中的PCB
	void Kill_Tree(PCB_Pointer PCB){
		//递归的杀死所有子进程
		for (auto begin = PCB->Creation_Tree->Children.begin(); begin != PCB->Creation_Tree->Children.end(); begin++) {
			Kill_Tree((*begin));
		}
		//将进程从对应队列中去掉
		if (PCB->status == ready) {
			if (PCB->Priority == user_) {
				for (auto begin = user_Ready_List.begin(); begin != user_Ready_List.end();begin++) {
					if ((*begin)->PID == PCB->PID) {
						user_Ready_List.erase(begin);
						break;
					}
				}
			}
			else if (PCB->Priority == system_) {
				for (auto begin = system_Ready_List.begin(); begin != system_Ready_List.end();begin++) {
					if ((*begin)->PID == PCB->PID) {
						system_Ready_List.erase(begin);
						break;
					}
				}
			}
		}
		else if (PCB->status == running) Running = nullptr;
		else if (PCB->status == blocked) {
			for (auto begin = Block_List.begin(); begin != Block_List.end();begin++) {
				if ((*begin)->PID == PCB->PID) {
					Block_List.erase(begin);
					break;
				}
			}
		}
		//归还进程占有的资源
		for (auto begin_ = PCB->Resource.begin(); begin_ != PCB->Resource.end(); begin_++) {
			RCB.plus_R((*begin_).RID, (*begin_).Count);
		}
		Scheduler();  //即使将阻塞的进程投入使用
	}
	//打印正在运行的程序
	void Print_Running_P() {
		cout <<"当前运行的程序是："<< Running->PID << endl;
	}
};

int main() {
	Test_Shell PM;   //建立资源管理程序PM
	string Order;   //命令
	string P_Name;  //进程名
	int Priority; //优先级
	char R;
	int Rid;  //资源id
	int N;   //资源数

	PM.init();   //初始化环境
	cout << "初始化完成\n";
	PM.Print_Running_P();
	cout << "请输入命令：(end 表示结束)\n";

	while (1) {
		cin >> Order;
		if (Order == "cr") {
			cin >> P_Name;
			cin >> Priority;
			PM.Create(P_Name, Priority);
		}
		else if (Order == "to") PM.Time_out();
		else if (Order == "req") {
			cin >> R;
			cin >> Rid;
			cin >> N;
			PM.Request(Rid, N);
		}
		else if (Order == "rel") {
			cin >> R;
			cin >> Rid;
			cin >> N;
			PM.Release(Rid, N);
		}
		else if (Order == "de") {
			cin >> P_Name;
			PM.Destroy(P_Name);
		}
		else if (Order == "end") {
			break;
		}
		PM.Print_Running_P();
	}

	return 0;
}