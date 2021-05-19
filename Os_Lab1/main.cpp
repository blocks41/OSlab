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

enum status_Type{ ready, running, blocked };   //���̵�״̬

struct PCB_Type;
using PCB_Pointer = struct PCB_Type*;
class RCB_Type;
struct Resource_Type;
struct Creation_Tree_Type;

//PCB�ṹ
struct PCB_Type {
	string PID;   //��������
	vector<Resource_Type> Resource;   //ռ�е���Դ����Դ��
	status_Type status;    //���̵�״̬
	Creation_Tree_Type* Creation_Tree;   //�������ṹ
	Prior_Type Priority;   //�������ȼ�
};
//RCB�ṹ
class RCB_Type {
private:
	int R[5] = { 0, 1, 2, 3, 4 };

public:
	friend class Test_Shell;

	int Get_RCB(int rid) {    //��ȡ������Դ��
		return R[rid];
	}
	void plus_R(int rid, int n) {   //������Դ��
		R[rid] += n;
	}
	void subs_R(int rid, int n) {   //������Դ��
		R[rid] -= n;
	}
};

//��Դ�ṹ
struct Resource_Type {
	int RID;   //��Դ����
	int Count;   //ռ����Դ��
};

struct Creation_Tree_Type {
	PCB_Pointer Parent;   //������
	list<PCB_Pointer> Children;   //�ӽ���
};

//���̹�����Test_Shell
class Test_Shell {   
private:
	//�������ȼ��ľ�������,���ﲻ����vector,��Ϊ����ҪƵ���Ĳ���ɾ��
	PCB_Pointer Running=nullptr;  //����ֻ����һ������
	list<PCB_Pointer> user_Ready_List;
	list<PCB_Pointer> system_Ready_List;
	list<PCB_Pointer> Block_List;  //��������
	RCB_Type RCB;   //��Դ���ƿ�

public:
	//��ʼ������
	void init() {
		//Init����������ʱ��������������������һ��ϵͳ���̻����û����̣�������˭�����������ߡ�
		PCB_Pointer PCB = new PCB_Type;
		PCB->Creation_Tree = new Creation_Tree_Type;
		PCB->PID = "init";   
		PCB->Priority = init_;  //���ȼ�Ϊ���
		PCB->status = running;
		Running = PCB;   //ֱ�ӷ�������
	}
	//��������
	void Create(string PID_, Prior_Type Priority_) { 
		//����PCB
		PCB_Pointer PCB=new PCB_Type;
		PCB->Creation_Tree = new Creation_Tree_Type;
		//��ʼ��PCB
		PCB->PID = PID_;
		PCB->Priority = Priority_;
		//���Ӹ��׽ڵ���ֵܽڵ�
		PCB->Creation_Tree->Parent = Running;
		if (Running) Running->Creation_Tree->Children.push_back(PCB);
		//��PCB�ӵ���Ӧ����������
		PCB->status = ready;
		if (PCB->Priority == user_) user_Ready_List.push_back(PCB);
		if (PCB->Priority == system_) system_Ready_List.push_back(PCB);
		//���̵���
		Scheduler();
	}
	//��������
	void Destroy(string pid) {
		PCB_Pointer PCB=nullptr;
		//�õ����̵�PCB�����ж�������
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
		//�����ý��̼����������
		if (PCB) {
			Kill_Tree(PCB);
			Scheduler();
		}
	}
	//������Դ
	void Request(int rid, int Num_of_Request) {   //������Դ
		int Num_of_Resource = RCB.Get_RCB(rid);   //��ȡ��Դʣ��
		//��PCB�м�����Ϣ 
		Resource_Type R;
		R.RID = rid;
		R.Count = Num_of_Request;
		Running->Resource.push_back(R);   

		if (Num_of_Resource >= Num_of_Request) {   //ʣ����Դ�����������
			RCB.subs_R(rid, Num_of_Request);      //��ȥ�������Դ
		}
		else
		{
			if (Num_of_Request > rid) {  //���������Դ��������Դ���������ʹ�ӡ�����˳�
				cout << "������Դ��������Դ������";
				exit(ERROR);
			}
			Running->status = blocked;
			Block_List.push_back(Running);  //������Դ������ʣ����Դ�����ͼ�����������
			//�����ж�����ɾ��
			Running = nullptr;
			Scheduler();
		}
	}
	//�ͷ���Դ
	void Release(int rid, int Num_of_Release) {
		//����Դr�ӵ�ǰ����ռ�õ���Դ�б����Ƴ���������Դr�Ŀ���������u��Ϊu+n
		for (auto begin = Running->Resource.begin(); begin != Running->Resource.end(); begin++) {
			if ((*begin).RID == rid) {
				RCB.plus_R(rid, Num_of_Release);
				Running->Resource.erase(begin);
				break;
			}
		}
		//�������ȼ�����ռʽ���Ȳ��ԣ���˵��н��̻����Դʱ����Ҫ�鿴��ǰ�����ȼ���������е���
		Scheduler(); 
	}
	//ʱ���ж�
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
	//����
	void Scheduler() {
		//����������в�Ϊ��
		for (auto begin = Block_List.begin(); begin != Block_List.end();) {
			int flag = 1;   //������־�Ƿ�������Դ����
			//�ȼ��
			for (int i = 0; i < (*begin)->Resource.size(); i++) {
				int rid = (*begin)->Resource[i].RID;//������Դid
				int Num_of_request = (*begin)->Resource[i].Count;  //������Դ��
				//����������Դ������ʣ�࣬��һ��ѭ��
				if (RCB.R[rid] < Num_of_request) {
					flag = 0;
					break;
				}
			}
			//�����Դ�����㣬�ͻ����������
			if (flag) {
				//������Դ
				for (int i = 0; i < (*begin)->Resource.size(); i++) {
					int rid = (*begin)->Resource[i].RID;//������Դid
					int Num_of_request = (*begin)->Resource[i].Count;  //������Դ��
					RCB.subs_R(rid, Num_of_request);
				}
				(*begin)->status = ready;
				// ���뵽��Ӧ��������
				if ((*begin)->Priority == user_) user_Ready_List.push_back((*begin));
				if ((*begin)->Priority == system_) system_Ready_List.push_back((*begin));
				Block_List.erase(begin); // ����Դr�������������Ƴ�
				begin = Block_List.begin();
			}
			else
			{
				begin++;
			}
		}
		//��û�����н���ʱ
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
		//����������ͷ�Ľ������ȼ����ߣ��ͽ��������еļ����������β
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
	//ɱ�����̣���ɾ�������е�PCB
	void Kill_Tree(PCB_Pointer PCB){
		//�ݹ��ɱ�������ӽ���
		for (auto begin = PCB->Creation_Tree->Children.begin(); begin != PCB->Creation_Tree->Children.end(); begin++) {
			Kill_Tree((*begin));
		}
		//�����̴Ӷ�Ӧ������ȥ��
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
		//�黹����ռ�е���Դ
		for (auto begin_ = PCB->Resource.begin(); begin_ != PCB->Resource.end(); begin_++) {
			RCB.plus_R((*begin_).RID, (*begin_).Count);
		}
		Scheduler();  //��ʹ�������Ľ���Ͷ��ʹ��
	}
	//��ӡ�������еĳ���
	void Print_Running_P() {
		cout <<"��ǰ���еĳ����ǣ�"<< Running->PID << endl;
	}
};

int main() {
	Test_Shell PM;   //������Դ�������PM
	string Order;   //����
	string P_Name;  //������
	int Priority; //���ȼ�
	char R;
	int Rid;  //��Դid
	int N;   //��Դ��

	PM.init();   //��ʼ������
	cout << "��ʼ�����\n";
	PM.Print_Running_P();
	cout << "���������(end ��ʾ����)\n";

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