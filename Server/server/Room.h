#ifndef ROOM_H
#define ROOM_H

#include <stdio.h>
#include <vector>
#include <map>
using namespace std;

class Room {
private:
	int player1;
	int player2;
	vector<int> viewer;
public:
	Room();
	void new_commer(int);
	vector<int> get_everyone();
	int get_player1() { return player1; }
	int get_player2() { return player2; }
};

class Rooms {
private:
	map<int, Room*> RoomList;	//RoomId
	map<int, Room*> RoomInfo; //ClientId
public:
	bool find_Room(int RoomId);
	bool join_Room(int ClientId, int RoomId);
	bool leave_Room(int ClientId);
	vector<int> get_everyone(int RoomId);
};


#endif // !ROOM_H