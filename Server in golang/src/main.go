package main

import (
	"awesomeProject/src/Room"
	"awesomeProject/src/pb"
	"container/list"
	"fmt"
	"github.com/golang/protobuf/proto"
	"net"
)

func (S *Server) initServer() {
	S.network = "tcp"
	S.address = "127.0.0.1:8888"
	S.unmsgbuf = make(map[string]string)
	S.clients = make(map[string]net.Conn)
	S.rooms.RoomList = make(map[int32]*Room.Room)
	S.rooms.RoomInfo = make(map[string]int32)
}

func main() {
	server := &Server{}
	server.initServer()
	listen, err := net.Listen(server.network, server.address) //监听
	if err != nil {
		fmt.Println("listen err=", err)
		return
	}
	fmt.Println("服务器开始监听。。。")
	defer listen.Close() //服务器结束前关闭 listener

	for { // 循环等待客户端来链接
		fmt.Println("等待客户端...")
		conn, err := listen.Accept() //创建用户数据通信的socket
		if err != nil {
			fmt.Println("Accept() err=", err)
		} else {
			fmt.Printf("主线程 %v 创建一个conn， Accept() suc con=%v 客户端 ip=%v\n", goID(), conn, conn.RemoteAddr().String())
			server.clients[conn.RemoteAddr().String()] = conn //存储客户端连接信息
		}
		//这里准备起一个协程，为客户端服务
		go server.process(conn.RemoteAddr().String())
	}
}

func (S *Server) process(ClientId string) {
	defer S.clients[ClientId].Close()
	for {
		//fmt.Printf("等待客户端%s 发送信息, 当前线程%v\n", ClientId, goID())
		var msgBuf list.List //多消息缓冲

		recv := S.recvMsg(ClientId, &msgBuf, &S.unmsgbuf) //接受消息
		if recv {                                         //接受消息成功，客户端没有离线
			for msg := msgBuf.Front(); msg != nil; msg = msg.Next() { //处理所有消息
				var baseMsg pb.BaseMsg //先检查收到的消息种类
				err := proto.Unmarshal([]byte(msg.Value.(string)), &baseMsg)
				if err != nil {
					fmt.Println("Unmarshal err")
					return
				} //反序列化出错

				switch baseMsg.MsgType { //筛选消息类型
				case pb.MsgType_MoveMsgType:
					{
						fmt.Printf("MoveMsg\n")
						S.moveMsgHandler(ClientId, msg.Value.(string))
					}
				case pb.MsgType_JoinRoomMsgType:
					{
						fmt.Printf("JoinRoomMsg\n")
						S.joinRoomMsgHandler(ClientId, msg.Value.(string))
					}
				case pb.MsgType_LeaveRoomMsgType:
					{
						fmt.Printf("LeaveRoomMsg\n")
						S.leaveRoomMsgHandler(ClientId, false)
					}
				case pb.MsgType_MessageMsgType:
					{
						fmt.Printf("MessageMsg\n")
						S.messageMsgHandler(ClientId, msg.Value.(string))
					}
				case pb.MsgType_RoomsMsgType:
					{
						fmt.Printf("RoomsMsg\n")
						S.roomsMsgHandler(ClientId)
					}
				case pb.MsgType_ChessMapMsgType:
					{
						fmt.Printf("ChessMapMsg\n")
						S.chessMapMsgHandler(ClientId, msg.Value.(string))
					}
				default:
				}
			}
		} else { //接受消息失败，客户端离线
			delete(S.clients, ClientId) //删除客户端的连接信息
			fmt.Printf("Client%s close\n", ClientId)
			S.leaveRoomMsgHandler(ClientId, true)
			break
		}
	}
}

func (S *Server) moveMsgHandler(ClientId string, msg string) {
	var moveMsg pb.MoveMsg
	err := proto.Unmarshal([]byte(msg), &moveMsg)
	if err != nil {
		fmt.Println("Unmarshal err")
		return
	}

	x := moveMsg.MoveX
	y := moveMsg.MoveY
	result := S.moveValidation(x, y, ClientId)
	if result != -2 { //通过有效性检查，发给房间内其他人
		fmt.Printf("Client%s wants:  (%d, %d)\n", ClientId, x, y)
		moveMsg.Validation = true
		S.sendToEveryoneInRoom(&moveMsg, ClientId, true) //直接转发move给每个用户,除了这个发来move的用户
		if result >= 0 {                                 //游戏结束，发送结果给房间内所有人
			var resultMsg pb.ResultMsg
			resultMsg.MsgType = pb.MsgType_ResultMsgType
			resultMsg.Color = result
			S.sendToEveryoneInRoom(&resultMsg, ClientId, false)
		}
	} else { //有效性检查失败，反馈给发来信息的客户端
		moveMsg.Validation = false
		sendBuf, err := proto.Marshal(&moveMsg)
		if err != nil {
			fmt.Println(",marshal err")
		}
		S.sendMsg(ClientId, string(sendBuf)) //直接发回
	}
}

func (S *Server) joinRoomMsgHandler(ClientId string, msg string) {
	var joinMsg pb.JoinRoomMsg
	err := proto.Unmarshal([]byte(msg), &joinMsg)
	if err != nil {
		fmt.Println("Unmarshal err")
		return
	}

	RoomId := joinMsg.RoomId
	fmt.Printf("Client%s join room%d\n", ClientId, RoomId)
	if S.rooms.JoinRoom(ClientId, RoomId) { //加入房间成功
		fmt.Printf("Client%s join room%d.\n", ClientId, int(joinMsg.RoomId))
		joinMsg.Join = true
		joinMsg.Player = S.rooms.IsPlayer(ClientId)
	} else if S.rooms.FindRoom(RoomId) { //client 已经加入某房间
		fmt.Printf("Client%s has already joined a room%d.\n", ClientId, int(joinMsg.RoomId))
		joinMsg.Join = false
		joinMsg.Player = false
	}
	sendBuf, err := proto.Marshal(&joinMsg)
	if err != nil {
		fmt.Println(",marshal err")
	}
	S.sendMsg(ClientId, string(sendBuf))

	if S.rooms.Count(RoomId) == 2 {
		S.rooms.RoomList[S.rooms.RoomInfo[ClientId]].Chess.EndChess() //重置棋盘
		S.startGame(ClientId)
	}
}

func (S Server) leaveRoomMsgHandler(ClientId string, disconnect bool) {
	opponent := S.rooms.GetOpponent(ClientId) //对手的ID
	if len(opponent) > 0 {                    //房间有对手: 告诉每个房间里的人上轮游戏结束
		S.endGame(ClientId, disconnect)
	}

	if !S.rooms.LeaveRoom(ClientId) { //离开房间失败
		fmt.Printf("Client%s not in room\n", ClientId)
	} else { //离开房间成功
		fmt.Printf("Client%s leave room.\n", ClientId)
		var leaveRoomMsg pb.LeaveRoomMsg
		leaveRoomMsg.MsgType = pb.MsgType_LeaveRoomMsgType
		leaveRoomMsg.ClientId = ClientId
		leaveRoomMsg.Leave = true
		S.sendToEveryoneInRoom(&leaveRoomMsg, opponent, false) //发给每个用户
		if !disconnect {                                       //Client只是离开房间， 不是断联
			sendBuf, err := proto.Marshal(&leaveRoomMsg)
			if err != nil {
				fmt.Println(",marshal err")
			}
			S.sendMsg(ClientId, string(sendBuf))
		}
	}
	opponent2 := S.rooms.GetOpponent(opponent) //对手的ID
	if len(opponent2) > 0 {                    //有观众成为新的player
		var joinMsg pb.JoinRoomMsg
		joinMsg.MsgType = pb.MsgType_JoinRoomMsgType
		joinMsg.Join = false
		joinMsg.Player = true
		sendBuf, err := proto.Marshal(&joinMsg)
		if err != nil {
			fmt.Println(",marshal err")
		}
		S.sendMsg(opponent2, string(sendBuf)) //告诉这个新的player
		S.startGame(opponent)                 //开始新的一轮游戏
	}

}

func (S Server) messageMsgHandler(ClientId string, msg string) {
	var messageMsg pb.MessageMsg
	err := proto.Unmarshal([]byte(msg), &messageMsg)
	if err != nil {
		fmt.Println("Unmarshal err")
		return
	}
	fmt.Printf("Client%s says:\t %s \n", ClientId, messageMsg.Message)
	messageMsg.ClientId = ClientId
	S.sendToEveryoneInRoom(&messageMsg, ClientId, true) //发给每个用户
}

func (S Server) roomsMsgHandler(ClientId string) {
	var roomsMsg = S.rooms.GetRoomsInfo()
	roomsMsg.MsgType = pb.MsgType_RoomsMsgType
	roomsMsg.ClientId = ClientId

	sendBuf, err := proto.Marshal(&roomsMsg)
	if err != nil {
		fmt.Println(",marshal err")
	}
	S.sendMsg(ClientId, string(sendBuf))
}

func (S Server) chessMapMsgHandler(ClientId string, msg string) {
	var chessMapMsg pb.ChessMapMsg
	err := proto.Unmarshal([]byte(msg), &chessMapMsg)
	if err != nil {
		fmt.Println("Unmarshal err")
		return
	}

	chessMapMsg.Step = S.rooms.GetRoom(ClientId).Chess.Step
	for i := 0; i < 15; i++ {
		for j := 0; j < 15; j++ {
			chessMapMsg.Map = append(chessMapMsg.Map, S.rooms.GetRoom(ClientId).Chess.MP[i][j])
			//fmt.Printf("%d", S.rooms.GetRoom(ClientId).Chess.MP[i][j])
		}
		//fmt.Printf("\n")
	}
	sendBuf, err := proto.Marshal(&chessMapMsg)
	if err != nil {
		fmt.Println(",marshal err")
	}
	S.sendMsg(ClientId, string(sendBuf))
}
