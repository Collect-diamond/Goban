
#include<atlstr.h>
#include<graphics.h>
#include<easyx.h>
#include<stdio.h>
#include<string.h>
#include<windows.h>
#include<time.h>
#include<iostream>
#include <mmsystem.h>
#include<dsound.h>
#pragma comment(lib, "WINMM.LIB")
#include"structure.h"
///////////////////////////////////////////////////////////////////
#define LOGOTEXT "GOBANG"
#define CS_E 0
#define CS_B 1
#define CS_W 2
#define IMG_LEN 46
#define MAP_LEN 15
#define DOT_LEN 3
#define MAX_SAVE 5
#define WIN_NUM 5
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
using namespace std;
///////////////////////////////////////////////////////////////////
typedef struct {
	int map[MAP_LEN][MAP_LEN];
	int num;
	int round;
	time_t save_time;
	time_t timelen;
	bool ai_spawn;
} SAVE;
///////////////////////////////////////////////////////////////////
typedef struct {
	float logoh;
	float logos;
	float logol;
} LOGO;
///////////////////////////////////////////////////////////////////
typedef struct {
	int _1_s;
	int _2_s;
	int _3_e1_s;
	int _3_e2_s;
	int _4_e1_s;
	int _4_e2_s;
	int _5_s;

	int _1_o;
	int _2_o;
	int _3_e1_o;
	int _3_e2_o;
	int _4_e1_o;
	int _4_e2_o;
	int _5_o;
} AI_BUF;
///////////////////////////////////////////////////////////////////
LPCTSTR chtocs(char* str) {
	CString cstr = str;
	LPCTSTR target = (LPCTSTR)cstr;
	return target;
}
///////////////////////////////////////////////////////////////////
void Timer(SAVE* save, int reset_q, int pause) {
	static time_t t2 = time(NULL);
	static time_t t_ = save->timelen;
	time_t t, dt;
	if (reset_q) {
		t2 = time(NULL);
		t_ = save->timelen;
	}
	t = time(NULL);
	dt = t - t2;
	if (!pause)
		save->timelen = t_ + dt;
}
///////////////////////////////////////////////////////////////////
void Sound_control() {
	void Sets_read(int* logo, int* vol, int* ai, int* theme);
	int tmp, volume;
	Sets_read(&tmp, &volume, &tmp, &tmp);
	int Vol = 333 * volume;
	char str01[60] = { "setaudio BackMusic volume to " },
		str02[60] = { "setaudio BackMusic2 volume to " };
	char vol_[10];
	itoa(Vol, vol_, 10);
	strcat(str01, vol_);
	strcat(str02, vol_);
	mciSendString(chtocs(str01), NULL, 0, NULL);
	mciSendString(chtocs(str02), NULL, 0, NULL);
}
///////////////////////////////////////////////////////////////////		//以下是对于五子棋的加权算法分析,分为多块,做详细描述
void AI_transfer(AI_BUF* buf, int choice) {
	switch (choice) {
	case 1: {
		buf->_1_s = 0, buf->_1_o = 1;
		buf->_2_s = 1; buf->_2_o = 2;
		buf->_3_e1_s = 5; buf->_3_e1_o = 8;
		buf->_3_e2_s = 10; buf->_3_e2_o = 10;
		buf->_4_e1_s = 10; buf->_4_e1_o = 20;
		buf->_4_e2_s = 80; buf->_4_e2_o = 100;
		buf->_5_s = 100; buf->_5_o = 200;
		break;
	}																	//简单AI权数分配
	case 2: {
		buf->_1_s = 0, buf->_1_o = 1;
		buf->_2_s = 1; buf->_2_o = 2;
		buf->_3_e1_s = 5; buf->_3_e1_o = 8;
		buf->_3_e2_s = 7; buf->_3_e2_o = 20;
		buf->_4_e1_s = 20; buf->_4_e1_o = 100;
		buf->_4_e2_s = 150; buf->_4_e2_o = 250;
		buf->_5_s = 300; buf->_5_o = 500;
		break;
	}																	//中等AI权数分配
	case 3: {
		buf->_1_s = 0, buf->_1_o = 1;
		buf->_2_s = 1; buf->_2_o = 2;
		buf->_3_e1_s = 5; buf->_3_e1_o = 8;
		buf->_3_e2_s = 10; buf->_3_e2_o = 30;
		buf->_4_e1_s = 20; buf->_4_e1_o = 50;
		buf->_4_e2_s = 100; buf->_4_e2_o = 200;
		buf->_5_s = 1000; buf->_5_o = 10000;
		break;
	}																	//困难AI权数分配
	default: {
		buf->_1_s = 0, buf->_1_o = 1;
		buf->_2_s = 1; buf->_2_o = 2;
		buf->_3_e1_s = 5; buf->_3_e1_o = 8;
		buf->_3_e2_s = 10; buf->_3_e2_o = 10;
		buf->_4_e1_s = 10; buf->_4_e1_o = 20;
		buf->_4_e2_s = 80; buf->_4_e2_o = 100;
		buf->_5_s = 100; buf->_5_o = 200;
		break;
	}																	//意外情况读取简单AI
	}
}
///////////////////////////////////////////////////////////////////
void AI_Tdata(SAVE* save, int x, int y, int i, int j, int* num, int* emp, int cs) {
	int tmp = 0, s = 0, t = 0, k = 0;
	//对玩家\AI落点评析
	for (tmp = 0; tmp < 2; tmp++) {
		if (!tmp) { s = 1, t = 1; }
		else { s = -1, t = -1; }
		for (k = s; s * k <= t * s * (WIN_NUM - 1); k += s)				//循环4次 + 反向判断
		{
			if (x + k * i >= 0 && x + k * i <= MAP_LEN - 1 &&
				y + k * j >= 0 && y + k * j <= MAP_LEN - 1 &&			//在无越界情况下存在黑子（玩家） 
				save->map[x + k * i][y + k * j] == cs)
				(*num)++;
			else if (save->map[x + k * i][y + k * j] == CS_E)			//判定空点
			{
				(*emp)++;
				break;
			}
			else														//如越界或者敌方棋子返回
				break;
		}
	}
}
///////////////////////////////////////////////////////////////////
void AI_buffer(int x, int y, int sum[MAP_LEN][MAP_LEN], int* cnt, int* emp, int player, int ai) {
	AI_BUF ai_buf;
	int _1, _2, _3_e1, _3_e2, _4_e1, _4_e2, _5;
	AI_transfer(&ai_buf, ai);											//读取AI权数数据
	if (player) {														//确定输入判定为玩家还是AI
		_1 = ai_buf._1_s, _2 = ai_buf._2_s;
		_3_e1 = ai_buf._3_e1_s, _3_e2 = ai_buf._3_e2_s;
		_4_e1 = ai_buf._4_e1_s, _4_e2 = ai_buf._4_e2_s;
		_5 = ai_buf._5_s;
	}																	//玩家权数读取
	else {
		_1 = ai_buf._1_o, _2 = ai_buf._2_o;
		_3_e1 = ai_buf._3_e1_o, _3_e2 = ai_buf._3_e2_o;
		_4_e1 = ai_buf._4_e1_o, _4_e2 = ai_buf._4_e2_o;
		_5 = ai_buf._5_o;
	}																	//AI权数读取
																		//计算空点权数
	if (*cnt == 0)													//周围只有单子
		sum[x][y] += _1;
	else if (*cnt == 1)												//周围存在2连
		sum[x][y] += _2;
	else if (*cnt == 2)												//周围存在3联
	{
		if (*emp == 1)
			sum[x][y] += _3_e1;										//死3 加分
		else if (*emp == 2)
			sum[x][y] += _3_e2;										//活3 加分
	}
	else if (*cnt == 3)												//周围存在4联
	{
		if (*emp == 1)
			sum[x][y] += _4_e1;										//死4 加分
		else if (*emp == 2)
			sum[x][y] += _4_e2;										//活4 加分
	}
	else if (*cnt >= 4)
		sum[x][y] += _5;											//即将5连，最高权
	*emp = 0;														//空点数量清零  

}
///////////////////////////////////////////////////////////////////
void Max_count(int sum[MAP_LEN][MAP_LEN], int* x, int* y) {
	int i, j, k = 0, tmpi, tmpj;
	long tmp = 0;
	int Pos[WIN_NUM * MAP_LEN][2];
	srand((unsigned)time(NULL));										//重置种子					
	for (i = 0; i < WIN_NUM * MAP_LEN; i++)
		Pos[i][0] = -1, Pos[i][1] = -1;
	tmp = sum[0][0];
	for (i = 0; i < MAP_LEN; i++)
		for (j = 0; j < MAP_LEN; j++)
			if (sum[i][j] > tmp) {
				tmp = sum[i][j];
				tmpi = i, tmpj = j;
			}
	for (i = 0; i < MAP_LEN; i++)
		for (j = 0; j < MAP_LEN; j++)
			if (tmp == sum[i][j]) {
				Pos[k][0] = i;
				Pos[k][1] = j;
				k++;
			}															//找出最大值的二维数组
	int random = rand() % k;											//挑选随机的一组最大值
	*x = Pos[random][0];												//posi赋值
	*y = Pos[random][1];												//posj赋值
}
///////////////////////////////////////////////////////////////////
void AI_judge(SAVE* save, int* posi, int* posj, int rnd_j, int ai) {
	srand((unsigned)time(NULL));
	int tp, tq;															//第一次落子时的x,y偏移量
	int x, y, t_i, t_j;													//计数器变量 
	int num_p = 0, num_o = 0;											//对双方连棋进行判定
	int emp = 0;														//空点数量判定变量
	static int sum[MAP_LEN][MAP_LEN] = { 0 };							//声明加权数组，静态方便提取
	if (rnd_j == 1) {
		for (x = 0; x < MAP_LEN; x++)
			for (y = 0; y < MAP_LEN; y++)
				if (save->map[x][y] == 1) {
					while (TRUE) {										//初次下子为特例，棋子随机在第一子半径为1之内寻找
						tp = rand() % 3 - 1, tq = rand() % 3 - 1;
						if (x + tp >= 0 && x + tp <= MAP_LEN - 1
							&& y + tq >= 0 && y + tq <= MAP_LEN - 1)
							if (save->map[x + tp][y + tq] == CS_E) {
								*posi = x + tp, * posj = y + tq;
								return;
							}
					}
				}
	}
	else {
		memset(sum, 0, sizeof(sum));									//sum加权数组预清空
		for (x = 0; x < MAP_LEN; x++) {									//posi遍历循环
			for (y = 0; y < MAP_LEN; y++) {								//posj遍历循环
				if (save->map[x][y] == CS_E) {							//如果这个点为空  
					for (t_i = -1; t_i <= 1; t_i++) {					//判断该点的延伸方向
						for (t_j = -1; t_j <= 1; t_j++) {
							if (t_i != 0 || t_j != 0) {					//排除棋盘判定原位9-1

								AI_Tdata(save, x, y, t_i, t_j, &num_p, &emp, CS_B);
								AI_buffer(x, y, sum, &num_p, &emp, 1, ai);
								//对玩家加权完毕
								AI_Tdata(save, x, y, t_i, t_j, &num_o, &emp, CS_W);
								AI_buffer(x, y, sum, &num_o, &emp, 0, ai);
								//对AI加权完毕
								num_p = 0, num_o = 0, emp = 0;			//累加变量清零
							}
						}
					}
				}
			}
		}
		Max_count(sum, posi, posj);										//计算权数最高点中的一点
		return;
	}
}
///////////////////////////////////////////////////////////////////		//AI设计结束
char* Preload_cs_pos(int posi, int posj, char* cs_pos) {
	switch (posi) {
	case 0: {
		switch (posj) {
		case 0:
		{strcpy(cs_pos, "lt"); break; }
		case MAP_LEN - 1:
		{strcpy(cs_pos, "lb"); break; }
		default:
		{strcpy(cs_pos, "l"); break; }
		}; break; }
	case MAP_LEN - 1: {
		switch (posj) {
		case 0:
		{strcpy(cs_pos, "rt"); break; }
		case MAP_LEN - 1:
		{strcpy(cs_pos, "rb"); break; }
		default:
		{strcpy(cs_pos, "r"); break; }
		}; break; }
	default: {
		switch (posj) {
		case 0:
		{strcpy(cs_pos, "t"); break; }
		case MAP_LEN - 1:
		{strcpy(cs_pos, "b"); break; }
		default:
		{strcpy(cs_pos, "m"); break; }
		}; break; }
	}
	switch (posi) {
	case DOT_LEN: {
		if (posj == DOT_LEN || posj == MAP_LEN - DOT_LEN - 1)
			strcpy(cs_pos, "o");
		break; }
	case MAP_LEN - DOT_LEN - 1: {
		if (posj == DOT_LEN || posj == MAP_LEN - DOT_LEN - 1)
			strcpy(cs_pos, "o");
		break; }
	case ((MAP_LEN - 1) / 2): {
		if (posj == ((MAP_LEN - 1) / 2))
			strcpy(cs_pos, "o");
		break; }
	}
	return cs_pos;
}
///////////////////////////////////////////////////////////////////
char* Preload_cs_stat(int stat, char* cs_stat) {
	switch (stat) {
	case CS_E:
	{*cs_stat = 'e'; cs_stat[1] = 0; break; }
	case CS_B:
	{*cs_stat = 'b'; cs_stat[1] = 0; break; }
	case CS_W:
	{*cs_stat = 'w'; cs_stat[1] = 0; break; }
	}
	return cs_stat;
}
///////////////////////////////////////////////////////////////////
char* Preload(int posi, int posj, int stat, char* tmp) {
	char* cs_pos, cs_stat[2];
	if ((cs_pos = (char*)malloc(40 * sizeof(char))) == NULL)
		exit(0);
	cs_pos[0] = 0; tmp[0] = 0;
	Preload_cs_pos(posi, posj, cs_pos);
	Preload_cs_stat(stat, cs_stat);
	strcat(tmp, "CS_DATA\\gb_res\\cs_");
	strcat(tmp, cs_stat);
	strcat(tmp, "_");
	strcat(tmp, cs_pos);
	strcat(tmp, ".jpg");
	free(cs_pos);
	return tmp;
}
///////////////////////////////////////////////////////////////////
void PlayMusic(const char input[], int pre) {
	char part1[50] = { "open " },
		part2[50] = { "play " },
		part3[50] = { "close " },
		part4[10] = { " wait" };
	strcat(part1, input);
	strcat(part2, input);
	strcat(part2, part4);
	strcat(part3, input);
	LPCTSTR cstr1 = chtocs(part1);
	mciSendString(cstr1, NULL, 0, NULL);
	if (pre) {
		LPCTSTR cstr2 = chtocs(part2);
		mciSendString(cstr2, NULL, 0, NULL);
	}
	LPCTSTR cstr3 = chtocs(part3);
	mciSendString(cstr3, NULL, 0, NULL);

}
///////////////////////////////////////////////////////////////////
void Load_cs(int posi, int posj, int(*map)[MAP_LEN], int stat) {
	char img_name[30]; int cs_data = *(*(map + posi) + posj);
	Preload(posi, posj, stat, img_name);
	IMAGE cs;
	CString cstr = img_name;
	loadimage(&cs, (LPCTSTR)cstr, IMG_LEN, IMG_LEN);
	putimage(posi * IMG_LEN, posj * IMG_LEN, &cs);
	*(*(map + posi) + posj) = stat;
}
///////////////////////////////////////////////////////////////////
void Reload(int(*map)[MAP_LEN], int clear) {
	void Map_generate(int(*map)[MAP_LEN]);
	int i = 0, posi, posj;
	if (clear)
		memset(map, 0, sizeof(map));
	Map_generate(map);
}
///////////////////////////////////////////////////////////////////
void Swapbkmusic() {
	static int num = 1;
	switch (num) {
	case 0: {
		mciSendString(L"close BackMusic", NULL, 0, NULL);
		mciSendString(L"open CS_DATA\\MusicResource\\bk_music_2.mp3 alias BackMusic2", NULL, 0, NULL);
		mciSendString(L"play BackMusic2 repeat", NULL, 0, NULL);
		Sound_control(); num = 1; break;
	}
	case 1: {
		mciSendString(L"close BackMusic2", NULL, 0, NULL);
		mciSendString(L"open CS_DATA\\MusicResource\\bk_music.mp3 alias BackMusic", NULL, 0, NULL);
		mciSendString(L"play BackMusic repeat", NULL, 0, NULL);
		Sound_control(); num = 0; break;
	}
	}
}
///////////////////////////////////////////////////////////////////
void Reset_map(SAVE* save) {
	memset(save->map, 0, sizeof(save->map));
	time_t t = time(NULL);
	save->save_time = t;
	save->num = 2;
	save->round = 0;
	save->timelen = 0;
	save->ai_spawn = 0;
}
///////////////////////////////////////////////////////////////////
void Loadj_Chess(int posi, int posj, SAVE* save) {
	if (*(*((save->map) + posi) + posj) == 0) {
		PlaySound(TEXT("CS_DATA\\MusicResource\\chess_put.wav"), NULL, SND_FILENAME | SND_ASYNC);
		Load_cs(posi, posj, save->map, ((save->round) % 2) + 1);
		(save->round)++;
	}
	else return;
}
///////////////////////////////////////////////////////////////////
void Chess_regret(SAVE* save, int i, int j, int reset) {
	static int d = save->round;
	if (reset) {
		d = save->round;
		return;
	}
	if (save->ai_spawn) {
		MessageBox(GetHWnd(), L"This option is not available in this mode", L"Sorry", MB_OK);
		return;
	}
	if (save->round == 0)
		return;
	if (!(save->ai_spawn)) {
		Load_cs(i, j, save->map, CS_E);
		if (d <= save->round) {
			PlaySound(TEXT("CS_DATA\\MusicResource\\removecs.wav"), NULL, SND_FILENAME | SND_ASYNC);
			d = save->round;
			save->round--;
		}
		return;
	}
}
///////////////////////////////////////////////////////////////////
void Save_rewrite(SAVE* save, SAVE* tar, int num) {
	SAVE tmp = *tar;
	*(save + num) = tmp;
	FILE* fp;
	fp = fopen("CS_DATA\\Saves\\save.dat", "wb");
	fwrite(save, sizeof(SAVE), MAX_SAVE, fp);
	fclose(fp);
}
///////////////////////////////////////////////////////////////////
void Save_inplay(SAVE* save) {
	void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]);
	void Save_read(SAVE * target);
	void Set_inplay(SAVE * save);
	IMAGE img;
	int i;
	char butf[128];
	time_t t; tm* local;
	CString local2;
	ExMessage m;
	SAVE tmp[MAX_SAVE];
	loadimage(&img, _T("CS_DATA\\Other resources\\save_inp.jpg"), 500, 600);
	putimage(250, 45, &img);
	for (i = 0; i < MAX_SAVE; i++)
		(tmp + i)->num = -1;
	Save_read(tmp);
	for (i = 0; i < MAX_SAVE; i++)
	{
		if ((tmp + i)->num == -1)
			Loadtext(250 + 100, 83 * i + 135, 20, BLACK, _T("黑体"), _T("(空)"));
		else {
			local = localtime(&(tmp[i].save_time));
			strftime(butf, 64, "%Y-%m-%d %H:%M:%S", local);
			local2 = butf;
			Loadtext(250 + 100, 83 * i + 135, 20, BLACK, _T("黑体"), local2);
		}
	}
	Loadtext(75, 615, 35, BLACK, _T("黑体"), _T("返回"));
	while (TRUE)
	{
		m = getmessage(EM_MOUSE | EM_KEY);
		if (m.message == WM_LBUTTONDOWN) {
			if (m.x >= 250 + 38 && m.x <= 250 + 318 && m.y >= 45 + 56 && m.y <= 45 + 120)
				Save_rewrite(tmp, save, 0), Save_inplay(save), Timer(save, 1, 0);
			if (m.x >= 250 + 38 && m.x <= 250 + 318 && m.y >= 45 + 138 && m.y <= 45 + 202)
				Save_rewrite(tmp, save, 1), Save_inplay(save), Timer(save, 1, 0);
			if (m.x >= 250 + 38 && m.x <= 250 + 318 && m.y >= 45 + 222 && m.y <= 45 + 284)
				Save_rewrite(tmp, save, 2), Save_inplay(save), Timer(save, 1, 0);
			if (m.x >= 250 + 38 && m.x <= 250 + 318 && m.y >= 45 + 304 && m.y <= 45 + 368)
				Save_rewrite(tmp, save, 3), Save_inplay(save), Timer(save, 1, 0);
			if (m.x >= 250 + 38 && m.x <= 250 + 318 && m.y >= 45 + 387 && m.y <= 45 + 452)
				Save_rewrite(tmp, save, 4), Save_inplay(save), Timer(save, 1, 0);
			if (m.x >= 250 + 38 && m.x <= 250 + 318 && m.y >= 45 + 496 && m.y <= 45 + 550)
				Set_inplay(save);
		}
	}
}
///////////////////////////////////////////////////////////////////
void Set_inplay(SAVE* save) {
	void Map_generate(int(*map)[MAP_LEN]);
	void Menu();
	void Getplaymsg(SAVE * save);
	IMAGE* img = new IMAGE[1];
	ExMessage m;
	loadimage(img, _T("CS_DATA\\Other resources\\sets_inp.jpg"));
	putimage(250, 45, img);
	delete[] img;
	while (TRUE)
	{
		m = getmessage(EM_MOUSE | EM_KEY);
		if (m.message == WM_LBUTTONDOWN) {
			if (m.x >= 250 + 185 && m.x <= 250 + 313 && m.y >= 45 + 263 && m.y <= 45 + 309)
			{
				Swapbkmusic();
			}
			if (m.x >= 250 + 185 && m.x <= 250 + 313 && m.y >= 45 + 338 && m.y <= 45 + 382)
				Save_inplay(save);
			if (m.x >= 250 + 185 && m.x <= 250 + 313 && m.y >= 45 + 412 && m.y <= 45 + 457)
				Timer(save, 1, 0), Map_generate(save->map), Getplaymsg(save);
			if (m.x >= 250 + 185 && m.x <= 250 + 313 && m.y >= 45 + 487 && m.y <= 45 + 532)
				Timer(save, 1, 0), Menu();
		}
	}
}
///////////////////////////////////////////////////////////////////
void Sumdata() {

}
///////////////////////////////////////////////////////////////////
void Play_modecse(SAVE* save) {
	void Menu();
	void Play(SAVE * save, bool ai_);
	ExMessage m;
	loadimage(NULL, _T("CS_DATA\\Other resources\\menu_2.jpg"));
	while (TRUE)
	{
		m = getmessage(EM_MOUSE | EM_KEY);
		if (m.message == WM_LBUTTONDOWN) {
			if (m.x >= 133 && m.x <= 426 && m.y >= 255 && m.y <= 447) {
				PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC);
				Reset_map(save), Timer(save, 1, 0); save->ai_spawn = 1, Play(save, 1);
			}
			if (m.x >= 551 && m.x <= 848 && m.y >= 251 && m.y <= 447) {
				PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC);
				mciSendString(L"close BackMusic", NULL, 0, NULL);
				mciSendString(L"open CS_DATA\\MusicResource\\bk_music_2.mp3 alias BackMusic2", NULL, 0, NULL);
				mciSendString(L"play BackMusic2 repeat", NULL, 0, NULL);
				Sound_control();
				Reset_map(save), Timer(save, 1, 0); save->ai_spawn = 0, Play(save, 0);
			}
			if (m.x >= 41 && m.x <= 212 && m.y >= 622 && m.y <= 662) {
				PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC);
				Menu();
			}
		}

	}
}
///////////////////////////////////////////////////////////////////
void Jump_gen(SAVE* save, int win) {
	void Load_timer(SAVE * save, int x, int y, int h);
	void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]);
	IMAGE img;
	loadimage(&img, _T("CS_DATA\\Other resources\\inplay_0.jpg"), 600, 500);
	putimage(200, 95, &img);
	Loadtext(388, 225, 30, BLACK, _T("黑体"), _T("用时:"));
	Load_timer(save, 475, 230, 25);
	if (win == -1) {
		PlaySound(TEXT("CS_DATA\\MusicResource\\chess_win.wav"), NULL, SND_FILENAME | SND_ASYNC);
		if (!(save->ai_spawn))
			Loadtext(383, 150, 50, BLACK, _T("黑体"), _T("玩家1胜利"));
		else
			Loadtext(413, 150, 50, BLACK, _T("黑体"), _T("你赢了!"));
	}
	if (win == 1) {
		if (!(save->ai_spawn)) {
			Loadtext(383, 150, 50, BLACK, _T("黑体"), _T("玩家2胜利"));
			PlaySound(TEXT("CS_DATA\\MusicResource\\chess_win.wav"), NULL, SND_FILENAME | SND_ASYNC);
		}
		else {
			Loadtext(413, 150, 50, BLACK, _T("黑体"), _T("你输了!"));
			PlaySound(TEXT("CS_DATA\\MusicResource\\game_fail.wav"), NULL, SND_FILENAME | SND_ASYNC);
		}
	}
	Loadtext(445, 333, 25, BLACK, _T("黑体"), _T("选取存档"));
	Loadtext(445, 408, 25, BLACK, _T("黑体"), _T("再来一局"));
	Loadtext(437, 483, 25, BLACK, _T("黑体"), _T("返回主菜单"));
}
///////////////////////////////////////////////////////////////////
void Jump_play(SAVE* save, int win) {
	void Menu();
	void Play(SAVE * save, bool ai_);
	long mx, my;
	ExMessage m;
	Timer(save, 1, 1);
	Jump_gen(save, win);
	while (TRUE)
	{
		void Save();
		int ai_tmp = 0;
		m = getmessage(EM_MOUSE | EM_KEY);
		mx = ((m.x) - 200) * 5 / 6, my = ((m.y) - 95) * 17 / 20;
		if (m.message == WM_LBUTTONDOWN) {
			if (mx >= 179 && mx <= 320 && my >= 190 && my <= 231)
				PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Save();
			if (mx >= 179 && mx <= 320 && my >= 253 && my <= 296) {
				PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC);
				ai_tmp = save->ai_spawn;
				Reset_map(save); Timer(save, 1, 0);
				save->ai_spawn = ai_tmp;
				Play(save, save->ai_spawn);
			}
			if (mx >= 179 && mx <= 320 && my >= 318 && my <= 359)
				PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Menu();
		}
	}
}
///////////////////////////////////////////////////////////////////
void Choose_play(ExMessage m, SAVE* save) {
	void cs_check(SAVE * save, int* win_);
	LPCTSTR test_tmp; char test[5];
	static int posi, posj;
	int cki, ckj, win = 0;
	if (m.x >= 0 && m.x <= 690 && m.y >= 0 && m.y <= 690) {
		posi = (int)(m.x) / IMG_LEN;
		posj = (int)(m.y) / IMG_LEN;
		cki = (IMG_LEN / 2) + IMG_LEN * posi;
		ckj = (IMG_LEN / 2) + IMG_LEN * posj;
		if (m.x >= cki - 21 && m.x <= cki + 21 && m.y >= ckj - 21 && m.y <= ckj + 21) {
			Loadj_Chess(posi, posj, save);
			cs_check(save, &win);
			if (win)
				Jump_play(save, win);
		}

	}
	else {
		if (m.x >= 700 && m.x <= 800 && m.y >= 500 && m.y <= 550)
			Chess_regret(save, posi, posj, 0);
		if (m.x >= 930 && m.x <= 1000 && m.y >= 0 && m.y <= 70)
			Set_inplay(save);
	}
}
///////////////////////////////////////////////////////////////////
void Getplaymsg(SAVE* save) {
	void cs_check(SAVE * save, int* win_);
	void Load_timer(SAVE * save, int x, int y, int h);
	void Load_playgen(SAVE * save);
	void Play_setgen(SAVE * save);
	void Load_selrnd(SAVE * save);
	void Sets_read(int* logo, int* vol, int* ai, int* theme);
	static int posi, posj;
	int tmp, ai, win = 0;
	IMAGE* img = new IMAGE[1];
	Sets_read(&tmp, &tmp, &ai, &tmp);
	BeginBatchDraw();
	Play_setgen(save);
	Load_selrnd(save);
	loadimage(img, _T("CS_DATA\\Other resources\\setstmp.jpg"), 120, 30);
	FlushBatchDraw();
	EndBatchDraw();
	ExMessage c_m;
	do {
		if (MouseHit()) {
			c_m = getmessage(EM_MOUSE | EM_KEY);
			if (c_m.message == WM_LBUTTONDOWN) {
				if (save->ai_spawn && (save->round) % 2 == 0)
					Choose_play(c_m, save);
				if (!(save->ai_spawn))
					Choose_play(c_m, save);
				if (save->ai_spawn && (save->round) % 2 != 0) {
					AI_judge(save, &posi, &posj, save->round, ai);
					Loadj_Chess(posi, posj, save);
					cs_check(save, &win);
					if (win)
						Jump_play(save, win);
				}
				BeginBatchDraw();
				Play_setgen(save);
				Load_selrnd(save);
				FlushBatchDraw();
				EndBatchDraw();
			}

			//Load_playgen(save);
		}
		BeginBatchDraw();
		putimage(775, 38, img);
		Timer(save, 0, 0);
		Load_timer(save, 780, 38, 27);
		FlushBatchDraw();
		EndBatchDraw();
	} while (TRUE);
	delete[] img;
}
///////////////////////////////////////////////////////////////////
void Map_generate(int(*map)[MAP_LEN]) {
	int i = 0, posi, posj;
	BeginBatchDraw();
	for (i = 0; i < MAP_LEN * MAP_LEN; i++) {
		posi = i / MAP_LEN, posj = i % MAP_LEN;
		Load_cs(posi, posj, map, *(*(map + posi) + posj));
	}
	FlushBatchDraw();
	EndBatchDraw();
}
///////////////////////////////////////////////////////////////////

void Drawrec_lens(int left, int top, int right, int bottom, int width) {
	int i = 0;
	for (i = 0; i < width; i++) {
		rectangle(left - i, top - i, right + i, bottom + i);
	}
}
void Drawpolybuf(int point1x, int point1y, int point2x, int point2y, int point3x, int point3y, int color) {
	POINT pts[] = { {point1x,point1y},{point2x,point2y},{point3x,point3y} };
	setfillcolor(color);
	setlinecolor(color);
	fillpolygon(pts, 3);
	setfillcolor(WHITE);
	setlinecolor(YELLOW);
}
void Load_selrnd(SAVE* save) {
	void Load_timer(SAVE * save, int x, int y, int h);
	void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]);
	IMAGE img1, img2, img3;
	int rnd = save->round + 1;
	char load[5];
	itoa(rnd, load, 10);
	Loadtext(950, 511, 30, BLACK, _T("黑体"), chtocs(load));
	setlinecolor(YELLOW);
	loadimage(&img1, _T("CS_DATA\\Other resources\\chs_b_show.jpg"), 30, 30);
	putimage(790, 150, &img1);
	if (!(save->round % 2)) {
		Drawrec_lens(789, 149, 822, 182, 3);
		Drawpolybuf(840, 165, 860, 161, 860, 170, BLACK);
	}
	if (save->ai_spawn) {
		loadimage(&img2, _T("CS_DATA\\Other resources\\chs_w_show.jpg"), 30, 30);
		putimage(790, 350, &img2);
		Loadtext(790, 250, 30, BLACK, _T("Arial Black"), _T("[Empty]"));
		if ((save->round % 2)) {
			Drawrec_lens(789, 349, 822, 382, 3);
			Drawpolybuf(840, 365, 860, 361, 860, 370, BLACK);
		}
	}
	else {
		loadimage(&img3, _T("CS_DATA\\Other resources\\chs_w_show.jpg"), 30, 30);
		putimage(790, 250, &img3);
		Loadtext(790, 350, 30, BLACK, _T("Arial Black"), _T("[Empty]"));
		if ((save->round % 2)) {
			Drawrec_lens(789, 249, 822, 282, 3);
			Drawpolybuf(840, 265, 860, 261, 860, 270, BLACK);
		}
	}
}
///////////////////////////////////////////////////////////////////
void Load_playgen(SAVE* save) {
	IMAGE img1, img2, img3;
	void Load_timer(SAVE * save, int x, int y, int h);
	void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]);
	void Sets_read(int* logo, int* vol, int* ai, int* theme);
	int ai, tmp, theme;
	Sets_read(&tmp, &tmp, &ai, &tmp);														//弃用函数
}
///////////////////////////////////////////////////////////////////
void Load_timer(SAVE* save, int x, int y, int h) {
	tm tmp; time_t tmp2;
	void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]);
	char timel[128];
	memset(&tmp, 0, sizeof(tm));
	tmp2 = (save->timelen);
	tmp.tm_sec = tmp2 % 60, tmp2 /= 60;
	tmp.tm_min = tmp2 % 60, tmp2 /= 60;
	tmp.tm_hour = tmp2 % 60, tmp2 /= 60;
	strftime(timel, 64, "%H:%M:%S", &tmp);
	Loadtext(x, y, h, BLACK, _T("黑体"), chtocs(timel));
}
///////////////////////////////////////////////////////////////////
void Play_setgen(SAVE* save) {
	void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]);
	IMAGE img, img2, img3;
	loadimage(&img, _T("CS_DATA\\Other resources\\setstmp.jpg"), 310, 690);
	putimage(691, 0, &img);
	loadimage(&img2, _T("CS_DATA\\Other resources\\set_p.jpg"), 70, 70);
	putimage(920, 10, &img2);
	loadimage(&img3, _T("CS_DATA\\Other resources\\menu.jpg"), 290, 115);
	putimage(700, 565, &img3);
	Loadtext(700, 35, 30, BLACK, _T("黑体"), _T("时间:"));
	Loadtext(700, 150, 30, BLACK, _T("黑体"), _T("玩家1:"));
	Loadtext(700, 250, 30, BLACK, _T("黑体"), _T("玩家2:"));
	Loadtext(700, 350, 30, BLACK, _T("黑体"), _T("AI::"));
	setlinecolor(BLACK);
	setfillcolor(RGB(176, 196, 222));
	fillrectangle(700, 500, 800, 550);
	Loadtext(721, 510, 30, DARKGRAY, _T("黑体"), _T("悔棋"));
	Loadtext(830, 510, 30, BLACK, _T("黑体"), _T("回合数:"));
}
///////////////////////////////////////////////////////////////////
void Play(SAVE* save, bool ai_) {
	initgraph(1000, 690);
	setbkcolor(RGB(255, 250, 205));
	cleardevice();
	IMAGE img;
	loadimage(&img, _T("CS_DATA\\Other resources\\setstmp.jpg"), 310, 690);
	putimage(691, 0, &img);
	void Sets_read(int* logo, int* vol, int* ai, int* theme);
	int logo, vol, ai, theme;
	Sets_read(&logo, &vol, &ai, &theme);
	Map_generate(save->map);
	Chess_regret(save, 0, 0, 1);
	Timer(save, 1, 0);
	Play_setgen(save);
	Load_playgen(save);
	Getplaymsg(save);
}
///////////////////////////////////////////////////////////////////
void Save_read(SAVE* target) {
	FILE* cs_sv;
	if ((cs_sv = fopen("CS_DATA\\Saves\\save.dat", "rb+")) == NULL) {
		if ((cs_sv = fopen("CS_DATA\\Saves\\save.dat", "ab+")) == NULL) {
			string folderpath = "CS_DATA\\Saves";
			string command;
			command = "mkdir " + folderpath;
			system(command.c_str());
			if ((cs_sv = fopen("CS_DATA\\Saves\\save.dat", "ab+")) == NULL) {
				MessageBox(GetHWnd(), L"Key data missing!", L"Data corrupted", MB_OK);
				exit(0);
			}
		}
	}
	int i = 0, j = 0;
	fread(target, sizeof(SAVE), MAX_SAVE, cs_sv);
	for (i = 0; i < MAX_SAVE; i++)
		if ((target + i)->num < 1 && (target + i)->num > 6) {
			j++;
			(target + i)->num = -1;
		}
	fclose(cs_sv);
}
///////////////////////////////////////////////////////////////////
void Save_Print(int n, SAVE* tmp) {
	time_t t; tm* local;
	CString local2;
	char butf[128];
	void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]);
	int i = 0;
	SAVE save[MAX_SAVE];
	for (i = 0; i < MAX_SAVE; i++)
		(save + i)->num = -1;
	Save_read(save);
	for (i = 0; i < MAX_SAVE; i++)
	{
		if ((save + i)->num == -1)
			Loadtext(100, 111 * i + 65, 23, BLACK, _T("黑体"), _T("(空)"));
		else {
			if (save[i].ai_spawn)
				Loadtext(340, 111 * i + 65, 23, BLACK, _T("黑体"), _T("[AI]"));
			else
				Loadtext(340, 111 * i + 65, 23, BLACK, _T("黑体"), _T("[PP]"));
			local = localtime(&(save[i].save_time));
			strftime(butf, 64, "%Y-%m-%d %H:%M:%S", local);
			local2 = butf;
			Loadtext(100, 111 * i + 65, 23, BLACK, _T("黑体"), local2);
		}
	}
	Loadtext(75, 615, 35, BLACK, _T("黑体"), _T("返回"));
}
///////////////////////////////////////////////////////////////////
void Save_judge(int i) {
	int j = 0;
	SAVE save[MAX_SAVE];
	SAVE tar;
	for (j = 0; j < MAX_SAVE; j++)
		(save + j)->num = -1;
	Save_read(save);
	if ((save + i)->num >= 1 && (save + i)->num <= MAX_SAVE) {
		tar = *(save + i);
		//delete[] save;
		Play(&tar, tar.ai_spawn);
	}
	else {
		MessageBox(GetHWnd(), L"It's an empty save file", L"Failed", MB_OK);
	}
}
///////////////////////////////////////////////////////////////////
void Save_scr() {
	SAVE tmp;
	initgraph(1000, 700);
	loadimage(NULL, _T("CS_DATA\\Other resources\\save_screen.jpg"), 1000, 700);
	Save_Print(0, &tmp);
}
///////////////////////////////////////////////////////////////////
void Choose_save(ExMessage m) {
	void Menu();
	int y = 110;
	if (m.x >= 50 && m.x <= 400 && m.y >= 25 && m.y <= y)
		PlaySound(TEXT("CS_DATA\\MusicResource\\Wrong.wav"), NULL, SND_FILENAME | SND_ASYNC), Save_judge(0);
	if (m.x >= 50 && m.x <= 400 && m.y >= (25 + 111 * 1) && m.y <= (y + 111 * 1))
		PlaySound(TEXT("CS_DATA\\MusicResource\\Wrong.wav"), NULL, SND_FILENAME | SND_ASYNC), Save_judge(1);
	if (m.x >= 50 && m.x <= 400 && m.y >= (25 + 111 * 2) && m.y <= (y + 111 * 2))
		PlaySound(TEXT("CS_DATA\\MusicResource\\Wrong.wav"), NULL, SND_FILENAME | SND_ASYNC), Save_judge(2);
	if (m.x >= 50 && m.x <= 400 && m.y >= (25 + 111 * 3) && m.y <= (y + 111 * 3))
		PlaySound(TEXT("CS_DATA\\MusicResource\\Wrong.wav"), NULL, SND_FILENAME | SND_ASYNC), Save_judge(3);
	if (m.x >= 50 && m.x <= 400 && m.y >= (25 + 111 * 4) && m.y <= (y + 111 * 4))
		PlaySound(TEXT("CS_DATA\\MusicResource\\Wrong.wav"), NULL, SND_FILENAME | SND_ASYNC), Save_judge(4);
	if (m.x >= 25 && m.x <= 185 && m.y >= (38 + 111 * 5) && m.y <= (y - 2 + 111 * 5))
		PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Menu();
}
///////////////////////////////////////////////////////////////////
void Save() {
	void Choose_menu(ExMessage m);
	Save_scr();
	ExMessage c_m;
	while (TRUE)
	{
		c_m = getmessage(EM_MOUSE | EM_KEY);
		if (c_m.message == WM_LBUTTONDOWN)
			Choose_save(c_m);
	}
}
///////////////////////////////////////////////////////////////////
int round_check(int(*map)[MAP_LEN], int posi, int posj) {
	switch (*(*(map + posi) + posj)) {
	case 1: return -1; break;
	case 2: return 1; break;
	}
	return 0;
}
///////////////////////////////////////////////////////////////////
int cs_check_1(int(*map)[MAP_LEN], int m, int n, int num) {
	int i = 0, cnt = 0;
	for (i = 0; i < 5; i++) {
		if (*(*(map + m) + n) == *(*(map + m - i * num) + n + i) && *(*(map + m) + n) != 0)
			cnt++;
		else return 0;
	}
	if (cnt == 5)
		return 1;
	return 0;
}
///////////////////////////////////////////////////////////////////
int cs_check_2(int(*map)[MAP_LEN], int m, int n, int num) {
	int i = 0, cnt = 0, num1, num2;
	if (num)
		num1 = 0, num2 = 1;
	else
		num1 = 1, num2 = 0;
	for (i = 0; i < 5; i++) {
		if (*(*(map + m) + n) == *(*(map + m + i * num1) + n + i * num2) && *(*(map + m) + n) != 0)
			cnt++;
		else return 0;
	}
	if (cnt == 5)
		return 1;
	return 0;
}
///////////////////////////////////////////////////////////////////
void cs_check(SAVE* save, int* win_) {
	int tmp;
	int i = 0, j = 0, k = 0, cnt_ = 0;
	for (k = 0; k < MAP_LEN - WIN_NUM + 1; k++)
		for (i = 0; i < MAP_LEN; i++)
			if (cs_check_2(save->map, k, i, 0) && save->map[k][i] != 0)
			{
				*win_ = round_check(save->map, k, i); return;
			}
	//posi->
	for (k = 0; k < MAP_LEN; k++)
		for (i = 0; i < MAP_LEN - WIN_NUM + 1; i++)
			if (cs_check_2(save->map, k, i, 1) && save->map[k][i] != 0)
			{
				*win_ = round_check(save->map, k, i); return;
			}
	//posj->
	for (k = WIN_NUM - 1; k < MAP_LEN; k++)
		for (i = 0; i < MAP_LEN - WIN_NUM + 1; i++)
			if (cs_check_1(save->map, k, i, 1) && save->map[k][i] != 0)
			{
				*win_ = round_check(save->map, k, i); return;
			}
	//pos /
	for (k = 0; k < MAP_LEN - WIN_NUM + 1; k++)
		for (i = 0; i < MAP_LEN - WIN_NUM + 1; i++)
			if (cs_check_1(save->map, k, i, -1) && save->map[k][i] != 0)
			{
				*win_ = round_check(save->map, k, i); return;
			}
	//pos "\"
}
///////////////////////////////////////////////////////////////////
void Loadtext(int x, int y, int h, int color, const wchar_t fon[], const wchar_t text[]) {
	LOGFONT font;
	setbkmode(TRANSPARENT);
	gettextstyle(&font);
	settextcolor(color);														// 获取当前字体设置
	font.lfHeight = h; //54														// 设置字体高度为 48
	_tcscpy(font.lfFaceName, (LPCTSTR)fon);										// 设置字体为"Hilda Header"
	font.lfQuality = ANTIALIASED_QUALITY;										// 设置输出效果为抗锯齿  
	settextstyle(&font);														// 设置字体样式
	outtextxy(x, y, (LPCTSTR)text);
}
///////////////////////////////////////////////////////////////////
void Load_menu() {
	initgraph(1000, 700);
	loadimage(NULL, _T("CS_DATA\\Other resources\\menu_3.jpg"));
	//Loadtext(50, 50, 150,BROWN, _T("华文行楷"), _T("五子棋"));
	Loadtext(50, 250, 54, BLACK, _T("黑体"), _T("开始游戏"));
	Loadtext(50, 350, 54, BLACK, _T("黑体"), _T("存档"));
	Loadtext(50, 450, 54, BLACK, _T("黑体"), _T("设置"));
	Loadtext(50, 550, 54, BLACK, _T("黑体"), _T("退出"));
}
///////////////////////////////////////////////////////////////////
void reset_save() {
	FILE* delsv_fp;
	delsv_fp = fopen("CS_DATA\\Saves\\save.txt", "w");
	rewind(delsv_fp);
	fclose(delsv_fp);
}
///////////////////////////////////////////////////////////////////
void Load_reset() {
	void Sets_load(int get1, int get2, int get3, int get4);
	void Settings(int Pic);
	void Sets_read(int* logo, int* vol, int* ai, int* theme);
	Sets_load(1, 2, 1, 1);
	reset_save();
	MessageBox(GetHWnd(), L"Reset complete", L"DONE", MB_OK);
	Settings(FALSE);
}
///////////////////////////////////////////////////////////////////
void Load_setscr(int pic) {
	void Load_setn();
	IMAGE img;
	if (pic)
		initgraph(1000, 700);
	BeginBatchDraw();
	loadimage(NULL, _T("CS_DATA\\Other resources\\menu.jpg"), 1000, 700);
	loadimage(&img, _T("CS_DATA\\Other resources\\sets.jpg"), 850, 550);
	putimage(75, 75, &img);
	Loadtext(875, 75, 75, BLACK, _T("微软雅黑"), _T("X"));
	Loadtext(100, 150, 27, BLACK, _T("黑体"), _T("启用/禁用LOGO："));
	Loadtext(100, 200, 27, BLACK, _T("黑体"), _T("音效："));
	Loadtext(100, 250, 27, BLACK, _T("黑体"), _T("AI难度："));
	Loadtext(100, 300, 27, BLACK, _T("黑体"), _T("主题："));
	Loadtext(100, 450, 27, BLACK, _T("黑体"), _T("版权说明：—— Y02114461ZXT"));
	Loadtext(100, 500, 27, BLACK, _T("黑体"), _T("重置设置[3]："));
	setlinecolor(BLACK);
	setfillcolor(RGB(176, 196, 222));
	fillrectangle(335, 500, 435, 525);
	Loadtext(343, 500, 27, BLACK, _T("Arial Black"), _T("DELETE"));
	Load_setn();
	FlushBatchDraw();
	EndBatchDraw();
}
///////////////////////////////////////////////////////////////////
void Sets_read(int* logo, int* vol, int* ai, int* theme) {
	void Settings(int Pic);
	FILE* cs_st;
	if ((cs_st = fopen("settings.txt", "r+")) == NULL) {
		if ((cs_st = fopen("settings.txt", "w")) == NULL) {
			MessageBox(GetHWnd(), L"Settings read failed", L"???", MB_OK);
			Settings(TRUE);
			return;
		}
		fprintf(cs_st, "general settings:\n");
		fprintf(cs_st, "Logo.T/F: 1"); fprintf(cs_st, "\n");
		fprintf(cs_st, "Volume: 2"); fprintf(cs_st, "\n");
		fprintf(cs_st, "AI: 1"); fprintf(cs_st, "\n");
		fprintf(cs_st, "Theme: 1"); fprintf(cs_st, "\n");
		rewind(cs_st);
	}
	char tmp[50]; int n;
	n = fscanf(cs_st, "%s%s", tmp, tmp);
	n = fscanf(cs_st, "%s%d", tmp, logo);
	n = fscanf(cs_st, "%s%d", tmp, vol);
	n = fscanf(cs_st, "%s%d", tmp, ai);
	n = fscanf(cs_st, "%s%d", tmp, theme);
	fclose(cs_st);
}
///////////////////////////////////////////////////////////////////
void Sets_cng(int i) {
	void Settings(int Pic);
	void Sets_load(int get1, int get2, int get3, int get4);
	int logo, vol, ai, theme;
	Sets_read(&logo, &vol, &ai, &theme);
	if (i == 1) {
		if (logo == 0)	logo = 1;
		else logo = 0;
	}
	if (i == 2) {
		if (vol != 3)	vol++;
		else vol = 0;
	}
	if (i == 3) {
		if (ai != 3)	ai++;
		else ai = 0 + 1;
	}
	if (i == 4) {
		if (theme == 0)	theme = 1;
		else theme = 0;
	}
	Sets_load(logo, vol, ai, theme);
	Sound_control();
	Settings(FALSE);
}
///////////////////////////////////////////////////////////////////
void Sets_load(int get1, int get2, int get3, int get4) {
	void Settings(int Pic);
	FILE* cs_st;
	if ((cs_st = fopen("settings.txt", "w")) == NULL)
		return;
	fprintf(cs_st, "general settings:\n");
	fprintf(cs_st, "Logo.T/F: %d", get1); fprintf(cs_st, "\n");
	fprintf(cs_st, "Volume: %d", get2); fprintf(cs_st, "\n");
	fprintf(cs_st, "AI: %d", get3); fprintf(cs_st, "\n");
	fprintf(cs_st, "Theme: %d", get4); fprintf(cs_st, "\n");
	fclose(cs_st);
}
///////////////////////////////////////////////////////////////////
void Load_setn() {
	int logo, vol, ai, theme;
	logo = 0, vol = 0, ai = 0, theme = 0;
	Sets_read(&logo, &vol, &ai, &theme);
	char logo1[4], theme1[10], ai1[10], vol1[10];
	if (logo) { strcpy(logo1, "ON"); }
	else { strcpy(logo1, "OFF"); }
	if (theme) { strcpy(theme1, "Default"); }
	else { strcpy(theme1, "Elder"); }
	switch (ai) {
	case 1:
	{ strcpy(ai1, "Normal"); break; }
	case 2:
	{ strcpy(ai1, "Medium"); break; }
	case 3:
	{ strcpy(ai1, "High"); break; }
	default:
	{ strcpy(ai1, "Normal"); break; }
	}
	switch (vol) {
	case 0:
	{ strcpy(vol1, "None"); break; }
	case 1:
	{ strcpy(vol1, "Low"); break; }
	case 2:
	{ strcpy(vol1, "Medium"); break; }
	case 3:
	{ strcpy(vol1, "High"); break; }
	default:
	{ strcpy(vol1, "None"); break; }
	}
	CString logo2 = logo1;
	CString vol2 = vol1;
	CString ai2 = ai1;
	CString theme2 = theme1;
	Loadtext(350, 150, 27, BLACK, _T("黑体"), logo2);
	Loadtext(350, 200, 27, BLACK, _T("黑体"), vol2);
	Loadtext(350, 250, 27, BLACK, _T("黑体"), ai2);
	Loadtext(350, 300, 27, BLACK, _T("黑体"), theme2);
}
///////////////////////////////////////////////////////////////////
void Choose_sets(ExMessage m) {
	int static cnt_ = 0;
	void Menu();
	if (m.x < 335 || m.x>435 || m.y < 500 || m.y>525) {
		cnt_ = 0;
		if (m.x >= 870 && m.x <= 925 && m.y >= 75 && m.y <= 75 + 60)
			PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Menu();//X
		if (m.x >= 350 - 5 && m.x <= 350 + 40 && m.y >= 150 && m.y <= 150 + 25)
			PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Sets_cng(1);
		if (m.x >= 350 - 5 && m.x <= 350 + 100 && m.y >= 200 && m.y <= 200 + 25)
			PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Sets_cng(2);
		if (m.x >= 350 - 5 && m.x <= 350 + 100 && m.y >= 250 && m.y <= 250 + 25)
			PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Sets_cng(3);
		if (m.x >= 350 - 5 && m.x <= 350 + 100 && m.y >= 300 && m.y <= 300 + 25)
			PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Sets_cng(4);
	}
	if (m.x >= 335 && m.x <= 435 && m.y >= 500 && m.y <= 525) {
		PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC);
		if (cnt_++ == 2)
			Load_reset(), cnt_ = 0;
	}
}
///////////////////////////////////////////////////////////////////
void Settings(int Pic) {
	Load_setscr(Pic);
	ExMessage c_m;
	while (TRUE)
	{
		c_m = getmessage(EM_MOUSE | EM_KEY);
		if (c_m.message == WM_LBUTTONDOWN)
			Choose_sets(c_m);
	}
}
///////////////////////////////////////////////////////////////////
void logo_scr() {
	LOGO exam; exam.logoh = 1, exam.logos = (float)0.7, exam.logol = (float)0.47;
	float logol2 = 1; float tmp = (float)-0.01;
	for (int i = 0; i < 2; i++) {
		do {
			setbkcolor(WHITE);
			LOGFONT font;
			setbkmode(TRANSPARENT);
			gettextstyle(&font);
			settextcolor(HSLtoRGB(exam.logoh, exam.logos, logol2));
			font.lfHeight = 108;
			_tcscpy(font.lfFaceName, _T("Arial Black"));
			font.lfQuality = ANTIALIASED_QUALITY;
			settextstyle(&font);
			outtextxy(300, 255, _T(LOGOTEXT));
			Sleep(20); logol2 += tmp;
		} while (logol2 >= exam.logol && logol2 <= 1);
		if (!i) Sleep(1250);
		tmp = -tmp;
	}
	Sleep(500);
}
///////////////////////////////////////////////////////////////////
void Logo(void) {
	int logo, a, b, c;
	Sets_read(&logo, &a, &b, &c);
	if (logo == FALSE)
		return;
	setbkcolor(WHITE);
	cleardevice();
	logo_scr();
}
///////////////////////////////////////////////////////////////////
void Choose_menu(ExMessage m) {
	void Menu();
	SAVE save; char i;
	if (m.x >= 50 && m.x <= 50 + 250 && m.y >= 250 - 10 && m.y <= 250 + 50)
		PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Play_modecse(&save);
	if (m.x >= 50 && m.x <= 50 + 125 && m.y >= 350 - 10 && m.y <= 350 + 50)
		PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Save();
	if (m.x >= 50 && m.x <= 50 + 125 && m.y >= 450 - 10 && m.y <= 450 + 50)
		PlaySound(TEXT("CS_DATA\\MusicResource\\btn.wav"), NULL, SND_FILENAME | SND_ASYNC), Settings(TRUE);
	if (m.x >= 50 && m.x <= 50 + 125 && m.y >= 550 - 10 && m.y <= 550 + 50)
		exit(0);
	if (m.x >= 940 && m.x <= 995 && m.y >= 640 && m.y <= 695) {
		loadimage(NULL, _T("CS_DATA\\Other resources\\rule.jpg"));
		while (TRUE)
		{
			m = getmessage(EM_MOUSE | EM_KEY);
			if (m.message == WM_LBUTTONDOWN)
				break;
		}
		Menu();
	}
}
///////////////////////////////////////////////////////////////////
void Menu() {
	mciSendString(L"close BackMusic2", NULL, 0, NULL);
	mciSendString(L"open CS_DATA\\MusicResource\\bk_music.mp3 alias BackMusic", NULL, 0, NULL);
	mciSendString(L"play BackMusic repeat", NULL, 0, NULL);
	Sound_control();
	Load_menu();
	SAVE tmp;
	tmp.timelen = 0;
	Timer(&tmp, 1, 0);
	ExMessage c_m;
	while (TRUE)
	{
		c_m = getmessage(EM_MOUSE | EM_KEY);
		if (c_m.message == WM_LBUTTONDOWN)
			Choose_menu(c_m);
	}
}
///////////////////////////////////////////////////////////////////
void File_check() {
	initgraph(1000, 700);
	FILE* fp;
	if ((fp = fopen("CS_DATA\\Other Resources\\Integrity.dat", "rb")) == NULL) {
		setbkcolor(WHITE);
		cleardevice();
		Loadtext(20, 20, 30, RGB(244, 100, 90), _T("Arial Black"), _T("Key source file missing！"));
		Loadtext(20, 70, 30, RGB(244, 100, 90), _T("Arial Black"), _T("Identify :Integrity.dat: failed!"));
		Loadtext(20, 120, 30, RGB(244, 100, 90), _T("Arial Black"), _T("Please check if file :CS_DATA: exist."));
		Loadtext(20, 170, 30, RGB(244, 100, 90), _T("Arial Black"), _T("Press any key to exit...	</>"));
		system("pause");
		exit(0);
	}
	else {
		fclose(fp);
	}
}
///////////////////////////////////////////////////////////////////		//前提性检查
void Show() {
	File_check();
	PlaySound(TEXT("CS_DATA\\MusicResource\\Empty.wav"), NULL, SND_FILENAME | SND_ASYNC);
	Logo();
	Menu();
}
///////////////////////////////////////////////////////////////////		//主函数
int main(void) {
	Show();
}