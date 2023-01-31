#include "room.h"

Room::Room(){
	player1 = -1;
	player2 = -1;
}

void Room::new_commer(int Id) {
	if (player1 < 0){
		player1 = Id;
	}
	else if (player2 < 0) {
		player2 = Id;
	}
	else {
		viewer.push_back(Id);
	}
}

vector<int> Room::get_everyone() {
	vector<int> idlist = viewer;
	idlist.push_back(player2);
	idlist.push_back(player1);
}


bool Rooms::find_Room(int RoomId){ //True 说明找到了room
	return (RoomList.find(RoomId) != RoomList.end());
}


bool Rooms::join_Room(int ClientId, int RoomId) {
	if (RoomInfo.find(ClientId) != RoomInfo.end()) {//这个client已经加入了room
		return false;
	}
	if (!find_Room(RoomId)) {//没找到room,该room未创建
		RoomList[RoomId] = new Room;
	}
	RoomInfo[ClientId] = RoomList[RoomId];
	return true;
}


bool Rooms::leave_Room(int ClientId) {
	if (RoomInfo.find(ClientId) == RoomInfo.end()) {//这个client没有加入了room
		return false;
	}
	RoomInfo.erase(ClientId);
	return true;
}


vector<int> Rooms::get_everyone(int RoomId) {//获得room里面所有人的id
	vector<int> idlist;
	if (find_Room(RoomId)) {
		idlist = RoomList[RoomId]->get_everyone();
	}
	return idlist;
}