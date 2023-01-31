#ifndef PUBLIC_H
#define PUBLIC_H

#include <winsock2.h>  
#include <stdio.h>
#include <vector>
#include <map>
#include <random>
#include "chess.h"
#include "server/msg/move.pb.h"
#include<ws2tcpip.h>//定义socklen_t
using namespace std;

#pragma comment(lib, "WS2_32")  // 链接到WS2_32.lib 

#define SERVER_IP "127.0.0.1"// 默认服务器端IP地址
#define SERVER_PORT 8888// 服务器端口号
#define  BUFFER_SIZE 50

class Room {
private:
	int roomId =-1;
	int player1 =-1;	// White
	int player2 =-1;	// Black
	vector<int> viewer;
	Chess chess;

public:
	Room() {};
	Room(int roomId) { this->roomId = roomId; }

	void new_commer(int);
	void leave(int);
	vector<int> get_everyone();

	int get_roomId() { return roomId; }
	int get_player1() { return player1; }
	int get_player2() { return player2; }
	bool swap_player() {
		if (player1 > 0 && player2 > 0) {
			int temp = player2; player2 = player1; player1 = temp;
			return true;
		}
		return false;
	}
	Chess* get_Chess() { return &chess; }
	bool is_player(int ClientId) { return (ClientId == player1 || ClientId == player2); }
	int count();//房间人数
};


class Rooms {
private:
	map<int, Room*> RoomList;	//RoomId
	map<int, Room*> RoomInfo; //ClientId
public:
	bool find_Room(int RoomId);
	bool join_Room(int ClientId, int RoomId);
	bool leave_Room(int ClientId);
	bool is_player(int ClientId);
	void print();
	Room* get_Room(int ClientId) {
		if (RoomInfo.find(ClientId) != RoomInfo.end()) {	//Client 在房间内
			return RoomInfo[ClientId];
		}
		return nullptr;
	}
	int get_opponent(int ClientId);
	int count(int);
	RoomsMsg get_roomsInfo();
	vector<int> get_everyone(int ClientId);
};


class Server{
public:
	Server();
	void Init();
	void Process();
	void Startgame(int);
	void Endgame(int);
	bool move_validate(int, int, int);

private:
	int listener;//监听套接字
	sockaddr_in  serverAddr;//IPV4的地址方式
	vector <int> socnum;//存放创建的套接字
	Rooms rooms;//存放所有的房间
};


string num2str(int i){
	char ss[10];
	sprintf(ss, "%04d", i);
	return ss;
}


void SendMsg(SOCKET s, string& sendbuf) {
	string length = num2str(sendbuf.length());
	//printf("send1: %s\n", sendbuf.c_str());
	//printf("before: %d\n", sendbuf.size());
	sendbuf = length + sendbuf; //将消息头加在前面
	//printf("send2: %s\n", sendbuf.c_str());
	//printf("after: %d\n", sendbuf.size()); //after = before+4
	int size = send(s, sendbuf.c_str(), sendbuf.size(), 0);
	//printf("Send size: %d\n", size);
}


int RecvMsg(SOCKET s, char* recvbuf, vector<string>& msgbuf, map<int, string>& unmsgbuf) {
	int size = recv(s, recvbuf, BUFFER_SIZE - 1, 0);
	//printf("Recv size: %d\t Recv: %s\n", size, recvbuf);
	//printf("Recv size: %d\n", size);
	if (size <= 0) { //对象断线
		return size;
	}
	//printf("Recv size: %d\n", size);
	string temp;
	temp.assign(recvbuf, size);
	string newmsg;
	string length;
	int len, i = 0;

	if (unmsgbuf[s].size() > 0) {//上个消息被分包了
		temp = unmsgbuf[s] + temp;
		size = temp.size();
		//printf("total size: %d\t Recv: %s\n", temp.size(), temp.c_str());
		//printf("total size: %d\n", temp.size());
		unmsgbuf.erase(s);
	}

	if (size >= 4) {	//处理粘包, 分包
		while (1) {
			length = temp.substr(i, 4);	//搜索消息头,得到消息长度
			//printf("Message length: %s\n", length.c_str());
			len = atoi(length.c_str());		//得到消息长度
			//printf("Message length: %d\n", len);
			newmsg = temp.substr(i + 4, len);	//搜索消息
			//printf("Real length: %d\n", newmsg.size());

			if (i + len + 4 == size) {//处理完所有信息.
				i += len + 4;
				msgbuf.push_back(newmsg);
				break;
			}
			else if (i + len + 4 <= size - 3) {//消息未被处理完,但能剩下一个包头的位置.
				i += len + 4;
				msgbuf.push_back(newmsg);
				continue;
			}
			else {//最后一个消息被分包了
				unmsgbuf[s] = temp.substr(i);
				break;
			}
		}
	}
	else {
		unmsgbuf[s] = temp.substr(i);
	}

	return size;
}





#endif // !PUBLIC_H