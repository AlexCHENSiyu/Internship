package main

import (
	"awesomeProject/src/Room"
	"awesomeProject/src/pb"
	"bytes"
	"container/list"
	"fmt"
	"github.com/golang/protobuf/proto"
	"math/rand"
	"net"
	"runtime"
	"strconv"
	"time"
)

const (
	BUFFERSIZE int32 = 1024
	WHITE_     int32 = 0
	BLACK_     int32 = 1
)

type Server struct {
	network  string
	address  string
	clients  map[string]net.Conn
	unmsgbuf map[string]string //未处理消息的缓冲
	rooms    Room.Rooms
}

func goID() uint64 {
	b := make([]byte, 64)
	b = b[:runtime.Stack(b, false)]
	b = bytes.TrimPrefix(b, []byte("goroutine "))
	b = b[:bytes.IndexByte(b, ' ')]
	n, _ := strconv.ParseUint(string(b), 10, 64)
	return n
}

func num2str(num int) string {
	str := fmt.Sprintf("%04d", num)
	return str
}

func (S *Server) sendMsg(ClientId string, sendBuf string) { // 回写数据给客户端
	length := num2str(len(sendBuf))
	sendBuf = length + sendBuf
	_, err := S.clients[ClientId].Write([]byte(sendBuf))
	if err != nil {
		fmt.Println("Write err:", err)
		return
	}
	//fmt.Printf("Send Size: %d\n", size)
}

func (S *Server) sendToEveryoneInRoom(m proto.Message, ClientId string, itself bool) {
	sendBuf, err := proto.Marshal(m)
	if err != nil {
		fmt.Println(",marshal err")
	}
	clientIdList := S.rooms.GetEveryone(ClientId)
	for clientId := clientIdList.Front(); clientId != nil; clientId = clientId.Next() {
		if itself && clientId.Value.(string) == ClientId {
			continue
		}
		//fmt.Printf("send to client%s\n", clientId.Value.(string))
		S.sendMsg(clientId.Value.(string), string(sendBuf))
	}
}

func (S *Server) recvMsg(ClientId string, msgBuf *list.List, unmsgbuf *map[string]string) bool {
	recvBuf := make([]byte, BUFFERSIZE) //接收数据的缓冲区

	size, err := S.clients[ClientId].Read(recvBuf) //从conn中读取客户端发送的数据内容
	if err != nil {                                //客户端掉线,接受失败
		//fmt.Printf("Client%s exit err=%v\n", ClientId, err)
		//fmt.Printf("Client%s exit\n", ClientId)
		return false //消息接受失败
	} else {
		//fmt.Printf("Recv size:%d\n", size)
	}

	//fmt.Printf("当前线程 %v, 接受消息 %s\n", goID(), string(recvBuf[:size]))
	var temp = string(recvBuf)
	var i = 0

	if len((*unmsgbuf)[ClientId]) > 0 { //上个消息被分包了
		temp = (*unmsgbuf)[ClientId] + temp
		size = len(temp)
		delete(*unmsgbuf, ClientId)
	}
	if size >= 4 {
		for {
			strlen := temp[i : i+4]           //搜索消息头,得到消息长度
			intLen, _ := strconv.Atoi(strlen) //得到消息长度
			//fmt.Printf("Msg lenth:%d\n", intLen)
			newmsg := temp[i+4 : i+4+intLen] //搜索消息

			if i+4+intLen == size { //处理完所有信息.
				i += intLen + 4
				msgBuf.PushBack(newmsg)
				break
			} else if i+4+intLen <= size-3 { //消息未被处理完,但能剩下一个包头的位置.
				i += intLen + 4
				msgBuf.PushBack(newmsg)
				continue
			} else { //最后一个消息被分包了
				(*unmsgbuf)[ClientId] = temp[i:]
				break
			}
		}
	} else {
		(*unmsgbuf)[ClientId] = temp[i:]
	}
	return true //消息接受成功
}

func (S *Server) moveValidation(x int32, y int32, ClientId string) int32 {
	roomId := S.rooms.RoomInfo[ClientId]
	room := S.rooms.RoomList[roomId]
	var result int32 = -2
	if room.Chess.GameEnd { //上把游戏结束了，需要重置棋盘
		room.Chess.EndChess()
	}

	inputSuccess := false
	if room.Player1 == ClientId {
		inputSuccess = room.Chess.GetInput(x, y, WHITE_)
	} else {
		inputSuccess = room.Chess.GetInput(x, y, BLACK_)
	}

	if inputSuccess { //输入成功
		result = room.Chess.Check()
		if result >= 0 { //游戏结束
			room.Chess.GameEnd = true
			if result == 0 {
				fmt.Printf("In Room%d: White Win!\n", roomId)
			} else if result == 1 {
				fmt.Printf("In Room%d: Black Win!\n", roomId)
			} else if result == 2 {
				fmt.Printf("In Room%d: Tie!\n", roomId)
			}
		}
	}
	return result //-2:未通过有效性检查; -1,0,1,2:通过有效性检查
}

func (S *Server) startGame(ClientId string) { //用户所在房间开始游戏
	room := S.rooms.GetRoom(ClientId)
	if room == nil {
		return
	}
	var orderMsg pb.OrderMsg
	orderMsg.MsgType = pb.MsgType_OrderMsgType
	orderMsg.Gamestart = true

	rand.Seed(time.Now().Unix())
	if rand.Int()%10 > 4 { //白棋(true)先行，黑棋(false)后行
		orderMsg.White = ClientId
		orderMsg.Black = S.rooms.GetOpponent(ClientId)
		if room.Player1 != ClientId { //房间存储颜色信息，player1 always white
			room.SwapPlayer() //交换双方棋色
		}
	} else {
		orderMsg.White = S.rooms.GetOpponent(ClientId)
		orderMsg.Black = ClientId
		if room.Player2 != ClientId { //房间存储颜色信息，player2 always black
			room.SwapPlayer() //交换双方棋色
		}
	}
	S.sendToEveryoneInRoom(&orderMsg, ClientId, false) //直接转发给每个用户
}

func (S *Server) endGame(ClientId string, disconnect bool) { //告诉所有人游戏结束
	S.rooms.GetRoom(ClientId).Chess.EndChess() //重置棋盘
	var orderMsg pb.OrderMsg
	orderMsg.MsgType = pb.MsgType_OrderMsgType
	orderMsg.Gamestart = false
	orderMsg.White = ""
	orderMsg.Black = ""
	S.sendToEveryoneInRoom(&orderMsg, ClientId, disconnect)
}
