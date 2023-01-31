#include "chess.h"


void Chess::gotoxy(const int x, const int y) {//������ƶ���(x, y)���� 
	COORD pos = { x,y };
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hOut, pos);
}


void Chess::draw_col(int x) {//������̵��м䲿�� (һ��)
	gotoxy(x, 1);
	cout << "��";
	for (int i = 2; i <= 14; i++) {
		gotoxy(x, i);
		cout << "��";
	}
	gotoxy(x, 15);
	cout << "��";
}


void Chess::draw_chess() {//��������
	system("cls");
	// 	for (int i = 2; i <= 30; i += 2) {//������ 
	// 		gotoxy(i, 0);
	// 		cout << i / 2;
	// 	}
	// 
	// 	for (int i = 1; i <= 15; i++) {//������ 
	// 		gotoxy(0, i);
	// 		cout << i;
	// 	}
	gotoxy(0, 0);
	cout << "������";

	gotoxy(2, 1);
	cout << "��";

	for (int i = 2; i <= 14; i++) {
		gotoxy(2, i);
		cout << "��";
	}

	gotoxy(2, 15);
	cout << "��";//���������� 

	for (int i = 4; i <= 28; i += 2) {
		draw_col(i);
	}//��һ��ѭ������������м䲿�� 

	gotoxy(30, 1);
	cout << "��";

	for (int i = 2; i <= 14; i++) {
		gotoxy(30, i);
		cout << "��";
	}

	gotoxy(30, 15);
	cout << "��";//��������Ҳ�

	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			gotoxy(i * 2+2, j+1);
			if (mp[i][j] == _WHITE) {
				cout << "��";//������� 
			}
			else if (mp[i][j] == _BLACK) {
				cout << "��";
			}
		}
	}

	gotoxy(35, 7);
	if (step % 2 == _WHITE) {
		cout << "�� TURN";
	}
	else {
		cout << "�� TURN";
	}
}


bool Chess::valid_input(const int x, const int y, const int color) {
	if (step % 2 != color && color != _EMPTY) {		//������ȷ���巽������,-1˵��������ɫ
		//gotoxy(35, 3);	printf("1\n");
		return false;
	}
	if ((x >= 15 || x < 0) //����
		|| (y >= 15 || y < 0)
		|| (mp[x][y] >= 0)) {//��λ����������
		//gotoxy(35, 3);	printf("2\n");
		return false;
	}
	//gotoxy(35, 3);	printf("3\n");
	return true;
}


bool Chess::get_input(const int x, const int y, const int color) {
	bool valid = valid_input(x, y, color);

	if (valid) {
		this->x = x;
		this->y = y;
	}
	//gotoxy(2, 30); printf("Take (%d, %d)", this->x, this->y);
	return valid;
}


int  Chess::check() {
	int result = -1;//-1 ��ζ��û�н���

	if (step % 2 == _WHITE) {//��ż�����ð��� 
		mp[x][y] = _WHITE;
		if ((mp[x + 1][y] == _WHITE && mp[x + 2][y] == _WHITE && mp[x + 3][y] == _WHITE && mp[x + 4][y] == _WHITE && x >= 0 && x < 11)
			|| (mp[x - 1][y] == _WHITE && mp[x + 1][y] == _WHITE && mp[x + 2][y] == _WHITE && mp[x + 3][y] == _WHITE && x >= 1 && x < 12)
			|| (mp[x - 2][y] == _WHITE && mp[x - 1][y] == _WHITE && mp[x + 1][y] == _WHITE && mp[x + 2][y] == _WHITE && x >= 2 && x < 13)
			|| (mp[x - 3][y] == _WHITE && mp[x - 2][y] == _WHITE && mp[x - 1][y] == _WHITE && mp[x + 1][y] == _WHITE && x >= 3 && x < 14)
			|| (mp[x - 4][y] == _WHITE && mp[x - 3][y] == _WHITE && mp[x - 2][y] == _WHITE && mp[x - 1][y] == _WHITE && x >= 4 && x < 15)
			|| (mp[x][y + 1] == _WHITE && mp[x][y + 2] == _WHITE && mp[x][y + 3] == _WHITE && mp[x][y + 4] == _WHITE && y >= 0 && y < 11)
			|| (mp[x][y - 1] == _WHITE && mp[x][y + 1] == _WHITE && mp[x][y + 2] == _WHITE && mp[x][y + 3] == _WHITE && y >= 1 && y < 12)
			|| (mp[x][y - 2] == _WHITE && mp[x][y - 1] == _WHITE && mp[x][y + 1] == _WHITE && mp[x][y + 2] == _WHITE && y >= 2 && y < 13)
			|| (mp[x][y - 3] == _WHITE && mp[x][y - 2] == _WHITE && mp[x][y - 1] == _WHITE && mp[x][y + 1] == _WHITE && y >= 3 && y < 14)
			|| (mp[x][y - 4] == _WHITE && mp[x][y - 3] == _WHITE && mp[x][y - 2] == _WHITE && mp[x][y - 1] == _WHITE && y >= 4 && y < 15)
			|| (mp[x + 1][y + 1] == _WHITE && mp[x + 2][y + 2] == _WHITE && mp[x + 3][y + 3] == _WHITE && mp[x + 4][y + 4] == _WHITE && x >= 0 && x < 11 && y >= 0 && y < 11)
			|| (mp[x - 1][y - 1] == _WHITE && mp[x + 1][y + 1] == _WHITE && mp[x + 2][y + 2] == _WHITE && mp[x + 3][y + 3] == _WHITE && x >= 1 && x < 12 && y >= 1 && y < 12)
			|| (mp[x - 2][y - 2] == _WHITE && mp[x - 1][y - 1] == _WHITE && mp[x + 1][y + 1] == _WHITE && mp[x + 2][y + 2] == _WHITE && x >= 2 && x < 13 && y >= 2 && y < 13)
			|| (mp[x - 3][y - 3] == _WHITE && mp[x - 2][y - 2] == _WHITE && mp[x - 1][y - 1] == _WHITE && mp[x + 1][y + 1] == _WHITE && x >= 3 && x < 14 && y >= 3 && y < 14)
			|| (mp[x - 4][y - 4] == _WHITE && mp[x - 3][y - 3] == _WHITE && mp[x - 2][y - 2] == _WHITE && mp[x - 1][y - 1] == _WHITE && x >= 4 && x < 15 && y >= 4 && y < 15)
			|| (mp[x - 1][y + 1] == _WHITE && mp[x - 2][y + 2] == _WHITE && mp[x - 3][y + 3] == _WHITE && mp[x - 4][y + 4] == _WHITE && x >= 4 && x < 15 && y >= 0 && y < 11)
			|| (mp[x + 1][y - 1] == _WHITE && mp[x - 1][y + 1] == _WHITE && mp[x - 2][y + 2] == _WHITE && mp[x - 3][y + 3] == _WHITE && x >= 3 && x < 14 && y >= 1 && y < 12)
			|| (mp[x + 2][y - 2] == _WHITE && mp[x + 1][y - 1] == _WHITE && mp[x - 1][y + 1] == _WHITE && mp[x - 2][y + 2] == _WHITE && x >= 2 && x < 13 && y >= 2 && y < 13)
			|| (mp[x + 3][y - 3] == _WHITE && mp[x + 2][y - 2] == _WHITE && mp[x + 1][y - 1] == _WHITE && mp[x - 1][y + 1] == _WHITE && x >= 1 && x < 12 && y >= 3 && y < 14)
			|| (mp[x + 4][y - 4] == _WHITE && mp[x + 3][y - 3] == _WHITE && mp[x + 2][y - 2] == _WHITE && mp[x + 1][y - 1] == _WHITE && x >= 0 && x < 11 && y >= 4 && y < 15)) {
			result = _WHITE; //0 for white
		}
	}
	else if (step % 2 == _BLACK) {//Ϊ�������ú��� 
		mp[x][y] = _BLACK;
		if ((mp[x + 1][y] == _BLACK && mp[x + 2][y] == _BLACK && mp[x + 3][y] == _BLACK && mp[x + 4][y] == _BLACK && x >= 0 && x < 11)
			|| (mp[x - 1][y] == _BLACK && mp[x + 1][y] == _BLACK && mp[x + 2][y] == _BLACK && mp[x + 3][y] == _BLACK && x >= 1 && x < 12)
			|| (mp[x - 2][y] == _BLACK && mp[x - 1][y] == _BLACK && mp[x + 1][y] == _BLACK && mp[x + 2][y] == _BLACK && x >= 2 && x < 13)
			|| (mp[x - 3][y] == _BLACK && mp[x - 2][y] == _BLACK && mp[x - 1][y] == _BLACK && mp[x + 1][y] == _BLACK && x >= 3 && x < 14)
			|| (mp[x - 4][y] == _BLACK && mp[x - 3][y] == _BLACK && mp[x - 2][y] == _BLACK && mp[x - 1][y] == _BLACK && x >= 4 && x < 15)
			|| (mp[x][y + 1] == _BLACK && mp[x][y + 2] == _BLACK && mp[x][y + 3] == _BLACK && mp[x][y + 4] == _BLACK && y >= 0 && y < 11)
			|| (mp[x][y - 1] == _BLACK && mp[x][y + 1] == _BLACK && mp[x][y + 2] == _BLACK && mp[x][y + 3] == _BLACK && y >= 1 && y < 12)
			|| (mp[x][y - 2] == _BLACK && mp[x][y - 1] == _BLACK && mp[x][y + 1] == _BLACK && mp[x][y + 2] == _BLACK && y >= 2 && y < 13)
			|| (mp[x][y - 3] == _BLACK && mp[x][y - 2] == _BLACK && mp[x][y - 1] == _BLACK && mp[x][y + 1] == _BLACK && y >= 3 && y < 14)
			|| (mp[x][y - 4] == _BLACK && mp[x][y - 3] == _BLACK && mp[x][y - 2] == _BLACK && mp[x][y - 1] == _BLACK && y >= 4 && y < 15)
			|| (mp[x + 1][y + 1] == _BLACK && mp[x + 2][y + 2] == _BLACK && mp[x + 3][y + 3] == _BLACK && mp[x + 4][y + 4] == _BLACK && x >= 0 && x < 11 && y >= 0 && y < 11)
			|| (mp[x - 1][y - 1] == _BLACK && mp[x + 1][y + 1] == _BLACK && mp[x + 2][y + 2] == _BLACK && mp[x + 3][y + 3] == _BLACK && x >= 1 && x < 12 && y >= 1 && y < 12)
			|| (mp[x - 2][y - 2] == _BLACK && mp[x - 1][y - 1] == _BLACK && mp[x + 1][y + 1] == _BLACK && mp[x + 2][y + 2] == _BLACK && x >= 2 && x < 13 && y >= 2 && y < 13)
			|| (mp[x - 3][y - 3] == _BLACK && mp[x - 2][y - 2] == _BLACK && mp[x - 1][y - 1] == _BLACK && mp[x + 1][y + 1] == _BLACK && x >= 3 && x < 14 && y >= 3 && y < 14)
			|| (mp[x - 4][y - 4] == _BLACK && mp[x - 3][y - 3] == _BLACK && mp[x - 2][y - 2] == _BLACK && mp[x - 1][y - 1] == _BLACK && x >= 4 && x < 15 && y >= 4 && y < 15)
			|| (mp[x - 1][y + 1] == _BLACK && mp[x - 2][y + 2] == _BLACK && mp[x - 3][y + 3] == _BLACK && mp[x - 4][y + 4] == _BLACK && x >= 4 && x < 15 && y >= 0 && y < 11)
			|| (mp[x + 1][y - 1] == _BLACK && mp[x - 1][y + 1] == _BLACK && mp[x - 2][y + 2] == _BLACK && mp[x - 3][y + 3] == _BLACK && x >= 3 && x < 14 && y >= 1 && y < 12)
			|| (mp[x + 2][y - 2] == _BLACK && mp[x + 1][y - 1] == _BLACK && mp[x - 1][y + 1] == _BLACK && mp[x - 2][y + 2] == _BLACK && x >= 2 && x < 13 && y >= 2 && y < 13)
			|| (mp[x + 3][y - 3] == _BLACK && mp[x + 2][y - 2] == _BLACK && mp[x + 1][y - 1] == _BLACK && mp[x - 1][y + 1] == _BLACK && x >= 1 && x < 12 && y >= 3 && y < 14)
			|| (mp[x + 4][y - 4] == _BLACK && mp[x + 3][y - 3] == _BLACK && mp[x + 2][y - 2] == _BLACK && mp[x + 1][y - 1] == _BLACK && x >= 0 && x < 11 && y >= 4 && y < 15)) {
			result = _BLACK; // 1 for black
		}
	}
	else if (step >= 15 * 15 - 1) {
		result = 2; //ƽ��
	}
	step++;
	return result;
}


void  Chess::undo() {//������һ��
	system("cls");
	mp[x][y] = _EMPTY;
	step--;
	draw_chess();
}


void Chess::end_chess() {
	for (int i = 0; i < 15; i++) { //map ���
		for (int j = 0; j < 15; j++) {
			mp[i][j] = _EMPTY;
		}
	}
	step = 0;
}


Chess::Chess() {
	for (int i = 0; i < 15; i++) { //map ���
		for (int j = 0; j < 15; j++) {
			mp[i][j] = _EMPTY;
		}
	}
	step = 0;
}