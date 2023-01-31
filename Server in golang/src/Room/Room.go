package Room

import (
	"awesomeProject/src/Chess"
	"awesomeProject/src/pb"
	"container/list"
	"fmt"
)

type Room struct {
	RoomId  int32       //房间号
	Player1 string      //白
	Player2 string      //黑
	Viewer  list.List   //观众
	Chess   Chess.Chess //棋盘
}

type Rooms struct {
	RoomList map[int32]*Room  //RoomId -> *Room
	RoomInfo map[string]int32 //ClientId -> RoomId
}

func (R *Room) SwapPlayer() bool {
	if len(R.Player1) > 0 && len(R.Player2) > 0 {
		temp := R.Player2
		R.Player2 = R.Player1
		R.Player1 = temp
		return true
	}
	return false
}

func (R *Room) IsPlayer(ClientId string) bool {
	return (ClientId == R.Player1) || (ClientId == R.Player2)
}

func (R *Room) Count() int32 {
	count := 0
	if len(R.Player1) != 0 {
		count++
	}
	if len(R.Player2) != 0 {
		count++
	}
	count += R.Viewer.Len()
	return int32(count)
}

func (R *Room) NewComer(ClientId string) {
	if len(R.Player1) <= 0 {
		R.Player1 = ClientId
		fmt.Printf("Client%s is Player1.\n", R.Player1)
	} else if len(R.Player2) <= 0 {
		R.Player2 = ClientId
		fmt.Printf("Client%s is Player2.\n", R.Player2)
	} else {
		R.Viewer.PushFront(ClientId)
		fmt.Printf("Client%s is Viewer.\n", ClientId)
	}
}

func (R *Room) Leave(ClientId string) {
	if R.Player1 == ClientId {
		R.Player1 = ""
		if R.Viewer.Len() > 0 {
			R.Player1 = R.Viewer.Front().Value.(string)
			R.Viewer.Remove(R.Viewer.Front())
		}
	} else if R.Player2 == ClientId {
		R.Player2 = ""
		if R.Viewer.Len() > 0 {
			R.Player2 = R.Viewer.Front().Value.(string)
			R.Viewer.Remove(R.Viewer.Front())
		}
	} else {
		for e := R.Viewer.Front(); e != nil; e = e.Next() {
			if ClientId == e.Value.(string) {
				R.Viewer.Remove(e)
				break
			}
		}
	}
}

func (R *Room) GetEveryone() list.List {
	var everyoneInfo list.List
	if len(R.Player1) > 0 {
		everyoneInfo.PushBack(R.Player1)
	}
	if len(R.Player2) > 0 {
		everyoneInfo.PushBack(R.Player2)
	}
	if R.Viewer.Len() > 0 {
		for viewerId := R.Viewer.Front(); viewerId != nil; viewerId = viewerId.Next() {

			everyoneInfo.PushBack(viewerId.Value.(string))
		}
	}
	return everyoneInfo
}

func (R *Rooms) FindRoom(RoomId int32) bool {
	_, ok := R.RoomList[RoomId]
	return ok
}

func (R *Rooms) FindClient(ClientId string) bool { //True: Client加入房间了
	_, ok := R.RoomInfo[ClientId]
	return ok
}

func (R *Rooms) Count(RoomId int32) int32 {
	if R.FindRoom(RoomId) {
		return R.RoomList[RoomId].Count()
	}
	return -1
}

func (R *Rooms) JoinRoom(ClientId string, RoomId int32) bool {
	if R.FindClient(ClientId) { //这个client已经加入了room
		return false
	}
	if !R.FindRoom(RoomId) { //没找到room,该room未创建
		R.RoomList[RoomId] = &Room{
			RoomId:  RoomId,
			Player1: "",
			Player2: ""}
		R.RoomList[RoomId].Chess.EndChess() //棋盘初始化
	}
	R.RoomList[RoomId].NewComer(ClientId)
	R.RoomInfo[ClientId] = RoomId
	R.Print()
	return true
}

func (R *Rooms) LeaveRoom(ClientId string) bool {
	if !R.FindClient(ClientId) { //这个client没有加入了room
		return false
	} else { //这个client加入了room
		RoomId := R.RoomInfo[ClientId]
		R.RoomList[RoomId].Leave(ClientId)
		if R.RoomList[RoomId].Count() == 0 { //房间没有人了：删除房间
			delete(R.RoomList, RoomId)
		}
		delete(R.RoomInfo, ClientId)
		return true
	}
}

func (R *Rooms) Print() {
	fmt.Printf("\n----------RoomList----------:\n")
	for k, v := range R.RoomList {
		fmt.Printf("Room%d (%d)\n", k, v.Count())
	}
	fmt.Printf("----------------------------:\n")
}

func (R *Rooms) GetEveryone(ClientId string) list.List {
	var ClientIdList list.List
	if R.FindClient(ClientId) { //这个client已经加入房间
		ClientIdList = R.RoomList[R.RoomInfo[ClientId]].GetEveryone()
	}
	return ClientIdList
}

func (R *Rooms) IsPlayer(ClientId string) bool {
	if R.FindClient(ClientId) {
		return R.RoomList[R.RoomInfo[ClientId]].IsPlayer(ClientId)
	}
	return false
}

func (R *Rooms) GetOpponent(ClientId string) string { //是player并且返回对手的Id
	if R.IsPlayer(ClientId) {
		RoomId := R.RoomInfo[ClientId]
		if ClientId == R.RoomList[RoomId].Player1 {
			return R.RoomList[RoomId].Player2
		} else if ClientId == R.RoomList[RoomId].Player2 {
			return R.RoomList[RoomId].Player1
		}
	}
	return ""
}

func (R *Rooms) GetRoomsInfo() pb.RoomsMsg {
	var roomsMsg pb.RoomsMsg
	roomsMsg.MsgType = pb.MsgType_RoomsMsgType
	for k, v := range R.RoomList {
		roomMsg := &pb.RoomMsg{}
		roomMsg.RoomId = k
		roomMsg.NumPeople = v.Count()
		roomsMsg.Rooms = append(roomsMsg.Rooms, roomMsg)
	}
	return roomsMsg
}

func (R *Rooms) GetRoom(ClientId string) *Room {
	if R.FindClient(ClientId) {
		return R.RoomList[R.RoomInfo[ClientId]]
	}
	return nil
}
