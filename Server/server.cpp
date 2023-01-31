#include "server/public.h"
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )


int main(){
	system("mode con cols=60 lines=40");
	Server ser;
	ser.Process();
	return 0;
}


Server::Server(){
	listener = 0;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = SERVER_PORT;
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);//将字符串类型转换uint32_t
}


void Server::Init(){//初始化函数，功能创建监听套接字，绑定端口，并进行监听
	int   Ret;
	WSADATA   wsaData; // 用于初始化套接字环境
	
	if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){// 初始化WinSock环境
		printf("WSAStartup()   failed   with   error   %d\n", Ret);
		WSACleanup();
	}

	listener = socket(AF_INET, SOCK_STREAM, 0);//采用ipv4,TCP传输
	if (listener == -1) {
		printf("Error at socket(): %ld\n", WSAGetLastError()); 
		perror("Listener failed"); exit(1);
	}
	printf("创建成功\n");

	unsigned long ul = 1;
	if (ioctlsocket(listener, FIONBIO, (unsigned long*)&ul) == -1) {//设置为非阻塞式
		perror("ioctl failed\n"); 
		exit(1); 
	};
	
	if (::bind(listener, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {//对socket进行地址和端口的绑定
		perror("Bind error\n");
		exit(1);
	}

	if (listen(listener, 10) < 0) {//开始监听
		perror("Listen failed\n"); 
		exit(1); 
	};
	socnum.push_back(listener);

	char Title[255];  itoa(listener, Title, 10);//控制窗口改名
	string strTitle(Title);
	strTitle = (string)"SERVER " + strTitle;
	SetConsoleTitle(strTitle.c_str());
}


void Server::Process(){
	fd_set fds;
	Init();
	printf("正在等待中\n");
	char recvbuf[BUFFER_SIZE];	//消息缓冲(所有客户端共用)
	string sendbuf;	//输出缓冲(所有客户端共用)
	vector<string> msgbuf;	//多消息缓冲(所有客户端共用)
	map<int, string> unmsgbuf;	//未被完全接受的消息缓冲

	while (1){
		FD_ZERO(&fds);//将fds清零
		for (int i = 0; i < socnum.size(); ++i){	//将fds每次都重新赋值
			FD_SET(socnum[i], &fds);
		}

		struct timeval  timeout = { 1,0 };//每个Select等待三秒
		switch (select(0, &fds, NULL, NULL, &timeout)){
			case -1:{
				printf("Select error at socket(): %ld\n", WSAGetLastError());
				printf("%d\n", socnum.size());
				//for (int i = 0; i < socnum.size(); ++i){printf("%d\n", socnum[i]);}
				Sleep(1000);
				break;
			}

			case 0:{
				//printf("select timeout......");
				break;
			}

			default:{//将数组中的每一个套接字都和剩余的额套接字进行比较得到当前的任务
				//server 的fd
				struct sockaddr_in  client_address;
				socklen_t client_addrLength = sizeof(struct sockaddr_in);
				int clientfd = accept(listener, (struct sockaddr*)&client_address, &client_addrLength);//返回一个用户的套接字
				if (clientfd > 0) { //添加用户，服务器上显示消息，并通知用户连接成功
					socnum.push_back(clientfd);
					printf("Client%d connect successfully\n", clientfd);
					printf("Clients number: %d\n", socnum.size() - 1);
				}
				
				for (auto i = socnum.begin(); i < socnum.end();){//如果第一个套接字可读的消息。就要建立连接
					if (i == socnum.begin()) {//server
						i++;
						continue;
					}

					else if (FD_ISSET((*i), &fds)){ //都是client的fd
						sendbuf.clear();//清空缓存
						msgbuf.clear();
						memset(recvbuf, '\0', BUFFER_SIZE);//清零

						int size = RecvMsg( (*i), recvbuf, msgbuf, unmsgbuf);
						//printf("size:%d\n",size);
						if (size == 0 || size == -1){//检测是否断线
							printf("Client%d close\n", (*i));
							vector<int> everyoneInfo = rooms.get_everyone(*i);
							int opponent = rooms.get_opponent(*i);//对手的ID
							if (opponent > 0) {//房间有人: 告诉每个用户上轮游戏结束
								Endgame(*i);
							}
							
							if (!rooms.leave_Room((*i))) { //这个人没有加入房间
								printf("Client%d not in any room.\n", (*i));
							}
							else {//通知所有人：有人离开了房间
								printf("Client%d leave room \n", (*i) );
								LeaveRoomMsg leaveroom_msg;
								leaveroom_msg.set_msg_type(LeaveRoomMsgType);
								leaveroom_msg.set_clientid(*i);
								leaveroom_msg.set_leave(TRUE);
								if (!leaveroom_msg.SerializeToString(&sendbuf)) {
									printf("Serialize fail\n");
									exit(1);
								}
								for (int j = 0; j < everyoneInfo.size(); ++j) {//告诉每个用户有人离开游戏
									SendMsg(everyoneInfo[j], sendbuf);
								}
							
								int opponent2 = rooms.get_opponent(opponent);//新对手的ID
								if (opponent2 > 0) {//有观众成为新的player
									JoinRoomMsg join_msg;
									join_msg.set_msg_type(JoinRoomMsgType);
									join_msg.set_join(FALSE);
									join_msg.set_player(TRUE);
									if (!join_msg.SerializeToString(&sendbuf)) {
										printf("Serialize fail\n");
										exit(1);
									}
									SendMsg(opponent2, sendbuf);
									Startgame(opponent);//开始新的一轮游戏
								}
							}
							//closesocket(socnum[i]);//先关闭这个套接字
							FD_CLR((*i), &fds);//在列表列表中删除
							i = socnum.erase(i);//在vector数组中删除

							continue;//直接下个循环
						}

						else{//若是没有掉线
							for (int m = 0; m < msgbuf.size(); m++) {//处理所有消息
								BaseMsg base_msg;//先检查收到的消息种类
								base_msg.ParseFromString(msgbuf[m]);

								switch (base_msg.msg_type())
								{
								case MoveMsgType: {
									//printf("MoveMsg\n");
									MoveMsg move_msg;
									move_msg.ParseFromString(msgbuf[m]);
									int x = move_msg.move_x();
									int y = move_msg.move_y();
									
									if (move_validate(x, y, (*i))) {//通过有效性检查，发给房间内其他人
										printf("Client%d wants:    (%d, %d)\n", (*i), x, y);
										move_msg.set_validation(TRUE);
										if (!move_msg.SerializeToString(&sendbuf)) {
											printf("Serialize fail\n");
											exit(1);
										}
										vector<int> everyoneInfo = rooms.get_everyone(*i);
										for (int j = 0; j < everyoneInfo.size(); ++j) {//直接转发给每个用户
											if (everyoneInfo[j] != (*i)) {
												SendMsg(everyoneInfo[j], sendbuf);
											}
										}
									}
									else {//有效性检查失败，反馈给发来信息的客户端
										move_msg.set_validation(FALSE);
										if (!move_msg.SerializeToString(&sendbuf)) {
											printf("Serialize fail\n");
											exit(1);
										}
										SendMsg((*i), sendbuf); //直接发回
									}
									
									break;
								}

								case JoinRoomMsgType: {
									//printf("JoinRoomMsg\n");
									JoinRoomMsg join_msg;
									join_msg.ParseFromString(msgbuf[m]);
									join_msg.set_msg_type(JoinRoomMsgType);
									if (rooms.join_Room((*i), join_msg.roomid())) {//加入房间
										printf("Client%d join room %d.\n", (*i), join_msg.roomid());
										join_msg.set_join(TRUE);
										join_msg.set_player(rooms.is_player((*i)));
									}
									else if (rooms.find_Room(join_msg.roomid())) {//client 已经加入某房间
										printf("Client%d has already joined a room.\n", (*i));
										join_msg.set_join(FALSE);
										join_msg.set_player(FALSE);
									}
									if (!join_msg.SerializeToString(&sendbuf)) {
										printf("Serialize fail\n");
										exit(1);
									}
									//printf("send JoinRoomMsg\n");
									SendMsg((*i), sendbuf);

									if (rooms.count(join_msg.roomid()) == 2) {//人数已满，此房间开始游戏
										Startgame((*i));
									}
									break;
								}

								case LeaveRoomMsgType: {
									printf("LeaveRoomMsg\n");
									LeaveRoomMsg leaveroom_msg;
									leaveroom_msg.ParseFromString(msgbuf[m]);
									vector<int> everyoneInfo = rooms.get_everyone(*i);

									int opponent = rooms.get_opponent(*i);//对手的ID
									if (opponent > 0) {//房间有人: 告诉每个用户上轮游戏结束
										Endgame(*i);
									}
									
									if (!rooms.leave_Room(*i)) {//离开房间
										printf("Client%d not in room\n", (*i));
									}
									else {
										printf("Client%d leave room.\n", (*i));
										leaveroom_msg.set_clientid((*i));
										leaveroom_msg.set_leave(TRUE);
										if (!leaveroom_msg.SerializeToString(&sendbuf)) {
											printf("Serialize fail\n");
											exit(1);
										}
										for (int j = 0; j < everyoneInfo.size(); ++j) {//直接转发给每个用户
											SendMsg(everyoneInfo[j], sendbuf);
										}
									}

									int opponent2 = rooms.get_opponent(opponent);//新player的ID
									if (opponent2 > 0) {//有观众成为新的player
										JoinRoomMsg join_msg;
										join_msg.set_msg_type(JoinRoomMsgType);
										join_msg.set_join(FALSE);
										join_msg.set_player(TRUE);
										if (!join_msg.SerializeToString(&sendbuf)) {
											printf("Serialize fail\n");
											exit(1);
										}
										SendMsg(opponent2, sendbuf);
										Startgame(opponent);//开始新的一轮游戏
									}
									break;
								}

								case MessageMsgType: {
									//printf("MessageMsg\n");
									MessageMsg message_msg;
									message_msg.ParseFromString(msgbuf[m]);
									printf("Client%d says:\t", (*i));
									printf(message_msg.message().c_str());
									printf("\n");

									message_msg.set_clientid((*i));
									if (!message_msg.SerializeToString(&sendbuf)) {
										printf("Serialize fail\n");
										exit(1);
									}
									vector<int>everyoneInfo = rooms.get_everyone(*i);
									for (int j = 0; j < everyoneInfo.size(); ++j) {//直接转发给每个用户
										if (everyoneInfo[j] != (*i)){
											SendMsg(everyoneInfo[j], sendbuf);
										}	
									}
									break;
								}

								case RoomsMsgType: {
									//printf("RoomsMsg\n");
									RoomsMsg rooms_msg = rooms.get_roomsInfo();
									rooms_msg.set_msg_type(RoomsMsgType);
									rooms_msg.set_clientid((*i));
									if (!rooms_msg.SerializeToString(&sendbuf)) {
										printf("Serialize fail\n");
										exit(1);
									}
									SendMsg((*i), sendbuf);
									break;
								}

								case ChessMapMsgType: {
									//printf("ChessMapMsgType\n");
									ChessMapMsg chessmap_msg;
									chessmap_msg.ParseFromString(msgbuf[m]);
									Chess* chess = rooms.get_Room(*i)->get_Chess();

									chessmap_msg.set_step(chess->get_step());
									chessmap_msg.set_msg_type(ChessMapMsgType);
									for (int m = 0;  m< 15; m++) {
										for (int n = 0; n < 15; n++) {
											chessmap_msg.add_map(chess->get_mp(m,n));
											//printf("%d ", chess->get_mp(m, n));
										}
										//printf("\n");
									}
									//printf("Size: %d\n", chessmap_msg.map_size());
									if (!chessmap_msg.SerializeToString(&sendbuf)) {
										printf("Serialize fail\n");
										exit(1);
									}
									//printf("%d", sendbuf.size());
									SendMsg((*i), sendbuf);
									break;
								}

								default:
									break;
								}
							}
							i++;	continue;
						}
					}
					i++;
				}
				break;
			}
		}
	}
}


void Server::Startgame(int clientId) {//用户所在房间开始游戏
	string sendbuf;
	unsigned seed = time(0);
	srand(seed);
	Room* room = rooms.get_Room(clientId);
	if (room == nullptr) return;

	OrderMsg order_msg;
	order_msg.set_msg_type(OrderMsgType);
	order_msg.set_gamestart(true);
	if (rand() % 10 > 4) {	//白棋(true)先行，黑棋(false)后行
		order_msg.set_white(clientId);
		order_msg.set_black(rooms.get_opponent(clientId));
		if (room->get_player1() != clientId) { //房间存储颜色信息，player1 always white
			room->swap_player();//交换双方棋色
		}
	}
	else {
		order_msg.set_white(rooms.get_opponent(clientId));
		order_msg.set_black((clientId));
		if (room->get_player2() != clientId) {	//房间存储颜色信息，player2 always black
			room->swap_player();//交换双方棋色
		}
	}
	if (!order_msg.SerializeToString(&sendbuf)) {
		printf("Serialize fail\n");
		exit(1);
	}
	vector<int> everyoneInfo = rooms.get_everyone(clientId);
	for (int j = 0; j < everyoneInfo.size(); ++j) {//直接转发给每个用户
		SendMsg(everyoneInfo[j], sendbuf);
	}
}


void Server::Endgame(int clientId) {//告诉所有人游戏结束
	rooms.get_Room(clientId)->get_Chess()->end_chess(); //重置棋盘

	string sendbuf;
	OrderMsg order_msg;
	order_msg.set_msg_type(OrderMsgType);
	order_msg.set_white(-1);
	order_msg.set_black(-1);
	order_msg.set_gamestart(false);
	if (!order_msg.SerializeToString(&sendbuf)) {
		printf("Serialize fail\n");
		exit(1);
	}
	vector<int> everyoneInfo = rooms.get_everyone(clientId);
	for (int j = 0; j < everyoneInfo.size(); ++j) {
		SendMsg(everyoneInfo[j], sendbuf);
	}
}


bool Server::move_validate(int x, int y, int ClientId) {
	Room* room = rooms.get_Room(ClientId);
	Chess* chess = room->get_Chess();
	bool input_success = false;
	if (room->get_player1() == ClientId) {	//In White side
		input_success = chess->get_input(x, y, _WHITE);
	}
	else {	//In Black side
		input_success = chess->get_input(x, y, _BLACK);
	}

	if (input_success) {//检查输入成功
		int result = chess->check();
		if (result >= 0) {//游戏结束
			if (result == 0) {//白棋获胜
				printf("In Room%d: White Win!\n", room->get_roomId());
				chess->end_chess();
			}
			else if (result == 1) {//黑棋获胜
				printf("In Room%d: Black Win!\n", room->get_roomId());
				chess->end_chess();
			}
			else if (result == 2) {//平局
				printf("In Room%d: Tie!\n", room->get_roomId());
				chess->end_chess();
			}
			string sendbuf;
			ResultMsg result_msg; //将服务端胜负信息发给每个客户端做确认
			result_msg.set_msg_type(ResultMsgType);
			result_msg.set_color(result);
			if (!result_msg.SerializeToString(&sendbuf)) {
				printf("Serialize fail\n");
				exit(1);
			}
			vector<int> everyoneInfo = rooms.get_everyone(ClientId);
			for (int j = 0; j < everyoneInfo.size(); ++j) {//直接转发给每个用户
				SendMsg(everyoneInfo[j], sendbuf);
			}
		}
		return true;
	}
	return false;
}


void Room::new_commer(int ClientId) {
	if (player1 < 0) {
		player1 = ClientId;
		printf("Client%d is a player1.\n",ClientId);
	}
	else if (player2 < 0) {
		player2 = ClientId;
		printf("Client%d is a player2.\n", ClientId);
	}
	else {
		viewer.push_back(ClientId);
		printf("Client%d is a viewer.\n",ClientId);
	}
}


void Room::leave(int ClientId) {
	if (player1 == ClientId) {
		player1 = -1;
		if (viewer.size() > 0) //让第一个观众变为玩家
		{
			player1 = *viewer.begin();
			printf("Client%d is a player1.\n", player1);
			viewer.erase(viewer.begin());
		}
	}
	else if (player2 == ClientId) {
		player2 = -1;
		if (viewer.size() > 0)//让第一个观众变为玩家
		{
			player2 = *viewer.begin();
			printf("Client%d is a player2.\n", player2);
			viewer.erase(viewer.begin());
		}
	} 
	else {
		for (int i = 0; i < viewer.size(); ++i) {
			if (viewer[i] == ClientId) {
				viewer.erase(viewer.begin() + i);
				break;
			}
		}
	}
}


vector<int> Room::get_everyone() {
	vector<int> ClientId_List(viewer);
	if (player1 > 0) ClientId_List.push_back(player1);
	if (player2 > 0) ClientId_List.push_back(player2);
	return ClientId_List;
}


int  Room::count() {
	int count = 0;
	if (player1 > 0) count++;
	if (player2 > 0) count++;
	count += viewer.size();
	return count;
}


bool Rooms::find_Room(int RoomId) { //True 说明找到了room
	return (RoomList.find(RoomId) != RoomList.end());
}


int Rooms::count(int RoomId) {
	if (RoomList.find(RoomId) != RoomList.end()) {//该room存在
		return RoomList[RoomId]->count();
	}
	return -1;
}


bool Rooms::join_Room(int ClientId, int RoomId) {
	if (RoomInfo.find(ClientId) != RoomInfo.end()) {//这个client已经加入了room
		return false;
	}
	if (!find_Room(RoomId)) {//没找到room,该room未创建
		RoomList[RoomId] = new Room(RoomId);
	}
	RoomList[RoomId]->new_commer(ClientId);
	RoomInfo[ClientId] = RoomList[RoomId];
	return true;
}


bool Rooms::leave_Room(int ClientId) {
	if (RoomInfo.find(ClientId) == RoomInfo.end()) {//这个client没有加入了room
		return false;
	}
	else {//这个client加入了room
		int roomId = RoomInfo[ClientId]->get_roomId();
		RoomInfo[ClientId]->leave(ClientId);
		if (RoomList[roomId]->count() == 0) {//房间没有人了：删除房间
			delete RoomList[roomId];
			RoomList.erase(roomId);
		}
		RoomInfo.erase(ClientId);
		return true;
	}
}


void Rooms::print() {
	for (map<int, Room*>::iterator i = RoomList.begin(); i != RoomList.end(); i++) {
		printf("Room%d (%d)\n", (*i).first, (*i).second->count());
	}
}


vector<int> Rooms::get_everyone(int ClientId) {//获得Client所在room里面所有人的id(包括自己)
	vector<int> clientId_List;
	if (RoomInfo.find(ClientId) != RoomInfo.end())//该client已经加入房间
	{
		clientId_List = RoomInfo[ClientId]->get_everyone();
	}
	return clientId_List;
}


bool Rooms::is_player(int ClientId) {
	if (RoomInfo.find(ClientId) != RoomInfo.end())//该client已经加入某房间
	{
		return RoomInfo[ClientId]->is_player(ClientId);
	}
	return false;//未加入房间
}


RoomsMsg  Rooms::get_roomsInfo() {
	RoomsMsg rooms_info;

	for (map<int, Room*>::iterator i = RoomList.begin(); i != RoomList.end(); i++) {
		RoomMsg* room;
		room = rooms_info.add_rooms();
		room->set_num_people((*i).second->count());
		room->set_roomid((*i).first);
	}
	return rooms_info;
}


int Rooms::get_opponent(int ClientId) {//是player并且返回对手的Id
	if (is_player(ClientId)) {
		if ( ClientId == RoomInfo[ClientId]->get_player1() ) {
			return RoomInfo[ClientId]->get_player2();
		}
		else if ( ClientId == RoomInfo[ClientId]->get_player2() ) {
			return RoomInfo[ClientId]->get_player1();
		}
	}
	return -1;
}