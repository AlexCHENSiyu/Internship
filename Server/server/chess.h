#pragma once
#ifndef CHESS_H
#define CHESS_H
#include <iostream>
#include<windows.h>
#include<conio.h>
#include<utility>
#include<ctime>
using namespace std;

#define _WHITE 0
#define  _BLACK 1
#define _EMPTY -1

class Chess {
private:
	int mp[15][15] = { _EMPTY };//��ͼ�����������������ɵ�
	int x = -1, y = -1;
	int step = 0;//�������Ҫ�������ж��Ǹð����߻��Ǻ����ߣ�ÿ������++��
								//ÿ���ж���ż�����ð��壻���������ú���.
	void draw_col(int);

public:
	Chess();
	void gotoxy(const int, const int);
	void draw_chess();
	bool valid_input(const int, const int, const int);
	bool get_input(const int, const int, const int);
	int check();
	void undo();
	void end_chess();
	int get_next_color() { return (step % 2); }
	int get_mp(const int m, const int n) { return mp[m][n]; }
	void set_mp(const int m, const int n, const int value) { mp[m][n] = value; }
	int get_step() { return step; }
	void set_step(const int s) { step = s; }
};


#endif