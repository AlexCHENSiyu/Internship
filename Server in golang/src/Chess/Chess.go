package Chess

const (
	WHITE_ int32 = 0
	BLACK_ int32 = 1
	EMPTY_ int32 = -1
)

type Chess struct {
	MP      [15][15]int32
	X       int32
	Y       int32
	Step    int32
	GameEnd bool
}

func (C *Chess) validInput(x int32, y int32, color int32) bool {
	if C.Step%2 != color && color != EMPTY_ {
		return false
	}
	if ((x >= 15) || (x < 0)) ||
		((y >= 15) || (y < 0)) ||
		(C.MP[x][y] >= 0) {
		return false
	}
	return true
}

func (C *Chess) GetInput(x int32, y int32, color int32) bool {
	valid := C.validInput(x, y, color)
	if valid {
		C.X = x
		C.Y = y
	}
	return valid
}

func (C *Chess) Check() int32 {
	var result int32 = -1
	mp := C.MP
	x := C.X
	y := C.Y
	color := C.Step % 2

	C.MP[C.X][C.Y] = color
	if C.Step >= 15*15-1 {
		result = 2 //平局
	} else if (x >= 0 && x < 11 && mp[x+1][y] == color && mp[x+2][y] == color && mp[x+3][y] == color && mp[x+4][y] == color) ||
		(x >= 1 && x < 12 && mp[x-1][y] == color && mp[x+1][y] == color && mp[x+2][y] == color && mp[x+3][y] == color) ||
		(x >= 2 && x < 13 && mp[x-2][y] == color && mp[x-1][y] == color && mp[x+1][y] == color && mp[x+2][y] == color) ||
		(x >= 3 && x < 14 && mp[x-3][y] == color && mp[x-2][y] == color && mp[x-1][y] == color && mp[x+1][y] == color) ||
		(x >= 4 && x < 15 && mp[x-4][y] == color && mp[x-3][y] == color && mp[x-2][y] == color && mp[x-1][y] == color) ||
		(y >= 0 && y < 11 && mp[x][y+1] == color && mp[x][y+2] == color && mp[x][y+3] == color && mp[x][y+4] == color) ||
		(y >= 1 && y < 12 && mp[x][y-1] == color && mp[x][y+1] == color && mp[x][y+2] == color && mp[x][y+3] == color) ||
		(y >= 2 && y < 13 && mp[x][y-2] == color && mp[x][y-1] == color && mp[x][y+1] == color && mp[x][y+2] == color) ||
		(y >= 3 && y < 14 && mp[x][y-3] == color && mp[x][y-2] == color && mp[x][y-1] == color && mp[x][y+1] == color) ||
		(y >= 4 && y < 15 && mp[x][y-4] == color && mp[x][y-3] == color && mp[x][y-2] == color && mp[x][y-1] == color) ||
		(x >= 0 && x < 11 && y >= 0 && y < 11 && mp[x+1][y+1] == color && mp[x+2][y+2] == color && mp[x+3][y+3] == color && mp[x+4][y+4] == color) ||
		(x >= 1 && x < 12 && y >= 1 && y < 12 && mp[x-1][y-1] == color && mp[x+1][y+1] == color && mp[x+2][y+2] == color && mp[x+3][y+3] == color) ||
		(x >= 2 && x < 13 && y >= 2 && y < 13 && mp[x-2][y-2] == color && mp[x-1][y-1] == color && mp[x+1][y+1] == color && mp[x+2][y+2] == color) ||
		(x >= 3 && x < 14 && y >= 3 && y < 14 && mp[x-3][y-3] == color && mp[x-2][y-2] == color && mp[x-1][y-1] == color && mp[x+1][y+1] == color) ||
		(x >= 4 && x < 15 && y >= 4 && y < 15 && mp[x-4][y-4] == color && mp[x-3][y-3] == color && mp[x-2][y-2] == color && mp[x-1][y-1] == color) ||
		(x >= 4 && x < 15 && y >= 0 && y < 11 && mp[x-1][y+1] == color && mp[x-2][y+2] == color && mp[x-3][y+3] == color && mp[x-4][y+4] == color) ||
		(x >= 3 && x < 14 && y >= 1 && y < 12 && mp[x+1][y-1] == color && mp[x-1][y+1] == color && mp[x-2][y+2] == color && mp[x-3][y+3] == color) ||
		(x >= 2 && x < 13 && y >= 2 && y < 13 && mp[x+2][y-2] == color && mp[x+1][y-1] == color && mp[x-1][y+1] == color && mp[x-2][y+2] == color) ||
		(x >= 1 && x < 12 && y >= 3 && y < 14 && mp[x+3][y-3] == color && mp[x+2][y-2] == color && mp[x+1][y-1] == color && mp[x-1][y+1] == color) ||
		(x >= 0 && x < 11 && y >= 4 && y < 15 && mp[x+4][y-4] == color && mp[x+3][y-3] == color && mp[x+2][y-2] == color && mp[x+1][y-1] == color) {
		result = color //0 for white
	}
	C.Step++
	return result
}

func (C *Chess) EndChess() {
	for i := 0; i < 15; i++ { //map 清空
		for j := 0; j < 15; j++ {
			C.MP[i][j] = EMPTY_
		}
	}
	C.X = EMPTY_
	C.Y = EMPTY_
	C.Step = 0
	C.GameEnd = false
}
