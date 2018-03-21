/*
 *        Computer Graphics Course - Shenzhen University
 *    Mid-term Assignment - Tetris implementation sample code
 * ============================================================
 *
 * - 本代码仅仅是参考代码，具体要求请参考作业说明，按照顺序逐步完成。
 * - 关于配置OpenGL开发环境、编译运行，请参考第一周实验课程相关文档。
 *
 * - 已实现功能如下：
 * - 1) 绘制棋盘格和‘L’型方块
 * - 2) 键盘左/右/下键控制方块的移动，上键旋转方块
 *
 * - 未实现功能如下：
 * - 1) 随机生成方块并赋上不同的颜色
 * - 2) 方块的自动向下移动
 * - 3) 方块之间的碰撞检测
 * - 4) 棋盘格中每一行填充满之后自动消除
 * - 5) 其他
 *
 * ============================================================
 * 助教：吴博剑(Bojian Wu)
 * 邮箱：bj.wu@siat.ac.cn
 * 如果对上述说明或作业有任何问题，欢迎发邮件咨询。 
 *
 * ============================================================
 * 致谢：参考代码来自于西蒙弗雷泽大学CMPT361-Assignment
 */

#include "include/Angel.h"

#pragma comment(lib, "glew32.lib")
#pragma warning(disable:4996)

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <time.h>
#include <string>
#include <algorithm>

using namespace std;

int starttime;			// 控制方块向下移动时间
int rotation = 0;		// 控制当前窗口中的方块旋转
vec2 tile[4];			// 表示当前窗口中的方块
bool gameover = false;	// 游戏结束控制变量

// 此处为了显示左右的信息，我对窗口大小进行了改动
int xsize = 700;		// 窗口大小（尽量不要变动窗口大小！）
int ysize = 700;

// 以下为新添加的全局变量
vec2 next_tile[4];			// 表示下一个窗口中的方块
int random_shape = 0;		// 随机数控制下落方块的随机的形状
int next_random_shape = 0;	// 随机数控制下一个方块的随机的形状
int clean_up_row = 0;		// 记录每一次被消除的行数
int score = 0;				// 记录游戏得分
char score_tip[100] = "score:";	// 界面输出所需
char next_tip[100] = "next:";	// 界面输出所需
char gameover_tip[100] = "";	// 界面输出所需
char score_out[100] = "0";		// 界面输出游戏得分
int score_rank[4] = { 0 };	// 记录游戏得分排行榜
bool al_update_rank = false;// 记录是否update了排行榜，防止重复update

GLuint program;		// 此处将着色器变为全局变量，以便使用

// 绘制窗口的颜色变量
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); // 橙色 1 (注释的编号代表数组下标)
vec4 white = vec4(1.0, 1.0, 1.0, 1.0);	// 白色
vec4 black = vec4(0.0, 0.0, 0.0, 1.0);	// 黑色
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);	// 绿色 2
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);	// 黄色 3
vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);	// 蓝色 4
vec4 purple = vec4(0.5, 0.0, 1.0, 1.0);	// 紫色 5
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);	// 红色 6

// 定义一个所有颜色的数组，方便下面做随机颜色的选择
vec4 all_color[] = { orange, green, yellow, blue, purple, red };
int random_color = 0;	// 定义一个随机数控制下落方块的随机的颜色
int next_random_color = 0;	// 定义下一个随机数控制下落方块的随机的颜色

// 当前方块的位置（以棋盘格的左下角为原点的坐标系）
vec2 tilepos = vec2(5, 19);
// 下一个方块的位置（以棋盘格的左下角为原点的坐标系）
vec2 next_tilepos = vec2(-3, 16);

bool isTheOne = true;	// 记录方块是不是游戏开始的第一个方块

// 布尔数组表示棋盘格的某位置是否被方块填充，即board[x][y] = true表示(x,y)处格子被填充。
// （以棋盘格的左下角为原点的坐标系）
bool board[10][20];

// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色
vec4 boardcolours[1500];  //1200
// 当棋盘格某些位置被方块填充之后，记录这些位置上被填充的颜色
vec4 next_tile_colours[50];  //48

GLuint locxsize;
GLuint locysize;

GLuint vaoIDs[3];
GLuint vboIDs[6];

// next 的VAO、VBO
GLuint next_vaoIDs;
GLuint next_vboIDs[2];

void showRank();	// 展示排行榜


// 一个二维数组表示所有可能出现的方块和方向。
//这个数组的设置比较难理解，是根据word文档里面的（下图右）里面的图示而来，老师上课会解释一下，请注意听

// 将原本的二维数组变成三维数组，表示7种形状以及它们的变形的形状
vec2 allRotationsLshape[7][4][4] =
{
	{
		{ vec2(0, 0), vec2(-1,0), vec2(1, 0), vec2(-1,-1) },	//   "L"
		{ vec2(0, 1), vec2(0, 0), vec2(0,-1), vec2(1, -1) },   //   
		{ vec2(1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0) },   //    
		{ vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1) }
	},
	{
		{ vec2(0, 0), vec2(1, 0), vec2(0, -1), vec2(-1, -1) },	//   "S"
		{ vec2(0, 0), vec2(1, 0), vec2(0,  1), vec2(1,  -1) },
		{ vec2(0, 0), vec2(1, 0), vec2(0, -1), vec2(-1, -1) },
		{ vec2(0, 0), vec2(1, 0), vec2(0,  1), vec2(1,  -1) }
	},
	{
		{ vec2(0, 0), vec2(-1, 0), vec2(0, -1), vec2(1, -1) },	//   "Z"
		{ vec2(0, 0), vec2(1, 0), vec2(1,  1), vec2(0,  -1) },
		{ vec2(0, 0), vec2(-1, 0), vec2(0, -1), vec2(1, -1) },
		{ vec2(0, 0), vec2(1, 0), vec2(1,  1), vec2(0,  -1) },
	},
	{
		{ vec2(0,  0), vec2(-1,0), vec2(1, 0), vec2(1, -1) },	//   "J"
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(1,  1) },
		{ vec2(-1, 1), vec2(-1,0), vec2(0, 0), vec2(1,  0) },
		{ vec2(-1,-1), vec2(0, 1), vec2(0, 0), vec2(0, -1) }
	},
	{
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },	//   "O"
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },
		{ vec2(0,  0), vec2(0,-1), vec2(-1, 0), vec2(-1, -1) },
	},
	{
		{ vec2(-2,  0), vec2(-1,0), vec2(0, 0), vec2(1, 0) },	//   "I"
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(0, -2) },
		{ vec2(-2,  0), vec2(-1,0), vec2(0, 0), vec2(1, 0) },
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(0, -2) },
	},
	{
		{ vec2(0,  0), vec2(-1,0), vec2(1, 0), vec2(0, -1) },	//   "T"
		{ vec2(0,  1), vec2(0, 0), vec2(0,-1), vec2(1,  0) },
		{ vec2(0,  1), vec2(-1,0), vec2(0, 0), vec2(1,  0) },
		{ vec2(-1, 0), vec2(0, 1), vec2(0, 0), vec2(0, -1) }
	}

};

//////////////////////////////////////////////////////////////////////////
// 修改棋盘格在pos位置的颜色为colour，并且更新对应的VBO

void changecellcolour(vec2 pos, vec4 colour)
{
	// 每个格子是个正方形，包含两个三角形，总共6个顶点，并在特定的位置赋上适当的颜色
	for (int i = 0; i < 6; i++)
		boardcolours[(int)(6*(10*pos.y + pos.x) + i)] = colour;

	vec4 newcolours[6] = {colour, colour, colour, colour, colour, colour};

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	// 计算偏移量，在适当的位置赋上颜色
	int offset = 6 * sizeof(vec4) * (int)(10*pos.y + pos.x);
	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//////////////////////////////////////////////////////////////////////////
// 当前方块移动或者旋转时，更新VBO

void updatetile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{
		// 计算格子的坐标值
		GLfloat x = tilepos.x + tile[i].x;
		GLfloat y = tilepos.y + tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// 每个格子包含两个三角形，所以有6个顶点坐标
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4};
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints);
	}
	glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////
// 我在此处添加了以下函数

// 更新下一个方块
void update_next_tile() {

	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[0]);

	// 每个方块包含四个格子
	for (int i = 0; i < 4; i++)
	{
		// 计算格子的坐标值
		GLfloat x = next_tilepos.x + next_tile[i].x;
		GLfloat y = next_tilepos.y + next_tile[i].y;

		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// 每个格子包含两个三角形，所以有6个顶点坐标
		vec4 newpoints[6] = { p1, p2, p3, p2, p3, p4 };
		glBufferSubData(GL_ARRAY_BUFFER, i * 6 * sizeof(vec4), 6 * sizeof(vec4), newpoints);
	}
	glBindVertexArray(0);
}

// 排序用的函数
bool compare(int a, int b)
{
	return a > b;
}
// 更新排行榜
void update_rank() {
	score_rank[3] = score;
	// 对分数排行榜进行排序
	sort(score_rank, score_rank + 4, compare);
	ofstream in;
	//ios::trunc表示在打开文件前将文件清空,由于是写入,文件不存在则创建
	in.open("score_rank.txt", ios::trunc);

	// 写入txt文本中记录
	for (int i = 0; i < 3; i++) {
		//cout << score_rank[i] << endl;
		if (i == 0) {
			in << score_rank[i];
		}
		else
			in << '\n' << score_rank[i];
	}
	// 展示更新后的排行榜
	showRank();
}

//////////////////////////////////////////////////////////////////////////
// 设置当前方块为下一个即将出现的方块。在游戏开始的时候调用来创建一个初始的方块，
// 在游戏结束的时候判断，没有足够的空间来生成新的方块。

// 此函数做了比较大的修改
void newtile()
{
	// 当棋盘顶格的位置被方块占着，游戏便结束
	if (board[5][19] || board[6][19]) {
		gameover = true;
		strcpy(gameover_tip, "gameover");
		glutSwapBuffers();
		// 如果未更新排行榜，便更新排行榜
		if (!al_update_rank) {
			update_rank();
			al_update_rank = true;
		}
		return;
	}

	srand(time(NULL));	// 设置随机数种子
	if (isTheOne) {
		// 产生0-6的随机数控制随机的形状
		random_shape = rand() % 7;
		// 产生0-5的随机数控制随机的颜色
		random_color = rand() % 6;
		isTheOne = false;
	}
	else {
		// 当前方块等于下一个方块
		random_shape = next_random_shape;
		random_color = next_random_color;
	}
	// 产生控制下一个方块的形状和颜色的随机数
	next_random_shape = rand() % 7;
	next_random_color = rand() % 6;

	// 将新方块放于棋盘格的最上行中间位置并设置默认的旋转方向
	tilepos = vec2(5, 19);
	rotation = 0;

	for (int i = 0; i < 4; i++)
	{
		// 这里的第一维的下标random_shape控制着随机形状
		tile[i] = allRotationsLshape[random_shape][0][i];
	}
	for (int i = 0; i < 4; i++)
	{
		// 这里的第一维的下标random_shape控制着随机形状
		next_tile[i] = allRotationsLshape[next_random_shape][0][i];
	}

	updatetile();			// 更新当前方块
	update_next_tile();		// 更新下一个方块

	// 给新方块赋上颜色
	vec4 newcolours[24];
	for (int i = 0; i < 24; i++)
		// 这里的下标random_color控制着随机颜色
		newcolours[i] = all_color[random_color];

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 给下一个新方块赋上颜色
	for (int i = 0; i < 24; i++)
		// 这里的下标random_color控制着随机颜色
		newcolours[i] = all_color[next_random_color];

	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
//////////////////////////////////////////////////////////////////////////
// 游戏和OpenGL初始化

void init()
{
	// 初始化棋盘格，包含64个顶点坐标（总共32条线），并且每个顶点一个颜色值
	vec4 gridpoints[64];
	vec4 gridcolours[64];

	// 纵向线
	for (int i = 0; i < 11; i++)
	{
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}

	// 水平线
	for (int i = 0; i < 21; i++)
	{
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}

	// 将所有线赋成白色
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;
	
	// 初始化棋盘格，并将没有被填充的格子设置成黑色
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black;

	// 对每个格子，初始化6个顶点，表示两个三角形，绘制一个正方形格子
	for (int i = 0; i < 20; i++)
		for (int j = 0; j < 10; j++)
		{
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);

			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}

	// 将棋盘格所有位置的填充与否都设置为false（没有被填充）
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false;

	// 载入着色器 (此处修改成全局变量)
	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	locxsize = glGetUniformLocation(program, "xsize");
	locysize = glGetUniformLocation(program, "ysize");

	GLuint vPosition = glGetAttribLocation(program, "vPosition");	// 获得顶点属性的入口的
	GLuint vColor = glGetAttribLocation(program, "vColor");			// 传递顶点数据	

	glGenVertexArrays(3, &vaoIDs[0]);	// 生成顶点数组
	
	// 棋盘格顶点
	glBindVertexArray(vaoIDs[0]);	// VAO 是一个对象，包含一或多个 VBO
	glGenBuffers(2, vboIDs);	// 生成新缓存对象

	// 棋盘格顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]);	// 绑定缓存对象
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW);; // 将顶点数据拷贝到缓存对象中
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 		// 传递顶点数据
	glEnableVertexAttribArray(vPosition);								// 传递顶点数据
	
	// 棋盘格顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	
	// 棋盘格每个格子	
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// 棋盘格每个格子顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 棋盘格每个格子顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	
	// 当前方块
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// 当前方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 当前方块顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// 此处添加了下一个方块的初始化
	// 下一个方块
	glBindVertexArray(next_vaoIDs);
	glGenBuffers(2, &next_vboIDs[0]);

	// 下一个方块顶点位置
	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// 下一个方块顶点颜色
	glBindBuffer(GL_ARRAY_BUFFER, next_vboIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);

	// 游戏初始化
	newtile();

	gameover = false;	// 添加判断游戏结束的标志
	score = 0;		// 初始化分数为0
	strcpy(score_out, "0");	// 初始化显示分数为0
	strcpy(gameover_tip, "");	// 初始化显示gameover的信息为空
	al_update_rank = false;		// 重新设置已更新排行榜标记

	// 从记录txt中读取分数排行榜
	ifstream in("score_rank.txt");
	string s;
	int i = 0;
	while (getline(in, s)) {
		//着行读取数据并存于s中，直至数据全部读取
		score_rank[i++] = stoi(s);
		if (i == 3)	break;
	}
	// glut的显示信息
	glutSwapBuffers();

	starttime = glutGet(GLUT_ELAPSED_TIME);
}

//////////////////////////////////////////////////////////////////////////
// 检查在cellpos位置的格子是否被填充或者是否在棋盘格的边界范围内。

// 此处添加一个条件使方块可以向上叠加
bool checkvalid(vec2 cellpos)
{
	// 此处添加一个条件使方块可以向上叠加
	if ((cellpos.x >= 0) && (cellpos.x < 10) && (cellpos.y >= 0) && (cellpos.y < 20)
		&& (board[(int)cellpos.x][(int)cellpos.y] == false))
		return true;
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
// 在棋盘上有足够空间的情况下旋转当前方块

// 此处添加了第一维的random_shape
void rotate()
{
	// 计算得到下一个旋转方向
	int nextrotation = (rotation + 1) % 4;

	// 检查当前旋转之后的位置的有效性
	// 此处添加了第一维的random_shape
	if (checkvalid((allRotationsLshape[random_shape][nextrotation][0]) + tilepos)
		&& checkvalid((allRotationsLshape[random_shape][nextrotation][1]) + tilepos)
		&& checkvalid((allRotationsLshape[random_shape][nextrotation][2]) + tilepos)
		&& checkvalid((allRotationsLshape[random_shape][nextrotation][3]) + tilepos))
	{
		// 更新旋转，将当前方块设置为旋转之后的方块
		rotation = nextrotation;
		for (int i = 0; i < 4; i++)
			// 此处添加了第一维的random_shape
			tile[i] = allRotationsLshape[random_shape][rotation][i];

		updatetile();
	}
}

//////////////////////////////////////////////////////////////////////////
// 检查棋盘格在row行有没有被填充满

// 此处自底向上遍历每一行，检查每一行的方格的标记情况
// 考虑存在多行同时被消除的情况
// 此处写成递归的形式比较简单
void checkfullrow(int row)
{
	// 如果最后一行就直接返回
	if (row == 19)return;
	// 自底向上检查每一行
	for (int i = 0; i < 10; i++) {
		if (board[i][row] == false) {
			// 检查上一行
			checkfullrow(row + 1);
			return;
		}
	}
	// 计算积分的标记++
	clean_up_row++;

	// 满足被clean的情况，进行消除
	// 消除就是自底向上依次把上一行的颜色和标记信息赋值给下一行
	for (int i = row; i < 19; i++) {
		for (int j = 0; j < 10; j++) {
			vec4 color = boardcolours[(int)(6 * (10 * (i + 1) + j))];
			changecellcolour(vec2(j, i), color);
			board[j][i] = board[j][i + 1];
		}
	}
	// 此时的当前行row就成为未被检查的那一行，于是继续递归
	checkfullrow(row);
}

//////////////////////////////////////////////////////////////////////////
// 放置当前方块，并且更新棋盘格对应位置顶点的颜色VBO

void settile()
{
	// 每个格子
	for (int i = 0; i < 4; i++)
	{
		// 获取格子在棋盘格上的坐标
		int x = (tile[i] + tilepos).x;
		int y = (tile[i] + tilepos).y;
		// 将格子对应在棋盘格上的位置设置为填充
		board[x][y] = true;
		// 并将相应位置的颜色修改
		changecellcolour(vec2(x, y), all_color[random_color]);	// 此处改为 random_color
	}
}

//////////////////////////////////////////////////////////////////////////
// 给定位置(x,y)，移动方块。有效的移动值为(-1,0)，(1,0)，(0,-1)，分别对应于向
// 左，向右和向下移动。如果移动成功，返回值为true，反之为false。

bool movetile(vec2 direction)
{
	// 计算移动之后的方块的位置坐标
	vec2 newtilepos[4];
	for (int i = 0; i < 4; i++)
		newtilepos[i] = tile[i] + tilepos + direction;

	// 检查移动之后的有效性
	if (checkvalid(newtilepos[0]) 
		&& checkvalid(newtilepos[1])
		&& checkvalid(newtilepos[2])
		&& checkvalid(newtilepos[3]))
		{
			// 有效：移动该方块
			tilepos.x = tilepos.x + direction.x;
			tilepos.y = tilepos.y + direction.y;

			updatetile();

			return true;
		}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// 重新启动游戏

void restart()
{
	// 检查是否更新排行榜，如果没有便更新
	if (!al_update_rank) {
		update_rank();
		al_update_rank = true;
	}
	init();	// 重新初始化即可
}

//////////////////////////////////////////////////////////////////////////
// 游戏渲染部分

// 此处增加了以下函数

// 此函数用于在屏幕上输出文字信息
void showText(float x, float y, const char* str, float rcolor, float gcolor, float bcolor) {
	glUseProgram(0);
	glColor3f(rcolor, gcolor, bcolor);	// 设置颜色
	glRasterPos2f(x, y);			// 用于显示[字体]时设置字符的起始位置
	int strLen = strlen(str);
	for (int i = 0; i<strLen; i++) {
		// 显示字符，字体-> GLUT_BITMAP_TIMES_ROMAN，字体大小-> 24
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
	}
	glFlush();
	glUseProgram(program);
}

// 此函数用于在屏幕上显示分数排行榜
void showRank() {
	showText(0.55, 0, "Score Rank:", 1.0, 1.0, 1.0);
	for (int i = 0; i < 3; i++) {
		char ss[100];
		char ii[100];
		char nn[100] = "NO.";
		itoa(i, ii, 10);
		strcat(nn, ii);
		itoa(score_rank[i], ss, 10);
		showText(0.55, -0.1 - i*0.1, nn, 1.0, 1.0, 1.0);
		showText(0.75, -0.1 - i*0.1, ss, 1.0, 1.0, 1.0);
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize);
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]);
	glDrawArrays(GL_TRIANGLES, 0, 1200); // 绘制棋盘格 (10*20*2 = 400 个三角形)

	glBindVertexArray(vaoIDs[2]);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制当前方块 (8 个三角形)

	glBindVertexArray(next_vaoIDs);
	glDrawArrays(GL_TRIANGLES, 0, 24);	 // 绘制下一个方块 (8 个三角形)

	glBindVertexArray(vaoIDs[0]);
	glDrawArrays(GL_LINES, 0, 64);		 // 绘制棋盘格的线

	// 此处showText把分数等文字信息显示在界面上
	showText(0.55, 0.75, score_tip, 1.0, 1.0, 1.0);
	showText(0.55, 0.65, score_out, 0.0, 1.0, 1.0);
	showText(0.55, 0.55, gameover_tip, 0.0, 1.0, 1.0);
	showText(-0.75, 0.75, next_tip, 1.0, 1.0, 1.0);

	// 显示分数排行榜
	showRank();

	glutSwapBuffers();  // 每次的所有绘图操作都在后台缓冲中进行， 当绘制完成时， 
						//把绘制的最终结果复制到屏幕上， 这样， 我们看到所有GDI元素同时出现在屏幕上
}

//////////////////////////////////////////////////////////////////////////
// 在窗口被拉伸的时候，控制棋盘格的大小，使之保持固定的比例。

void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//////////////////////////////////////////////////////////////////////////
// 键盘响应事件中的特殊按键响应

void special(int key, int x, int y)
{
	if (!gameover)
	{
		switch (key)
		{
		case GLUT_KEY_UP:	// 向上按键旋转方块
			rotate();
			break;
		case GLUT_KEY_DOWN: // 向下按键移动方块
			if (!movetile(vec2(0, -1)))
			{
				settile();
				checkfullrow(0);	// 检查消除情况
				// 按照积分计算规则计算积分
				score += 5 * clean_up_row * (clean_up_row + 1);
				sprintf(score_out, "%d", score);	// 将积分赋值给buffer
				glutSwapBuffers();		// 刷新界面
				clean_up_row = 0;		// 重置clearupflag
				newtile();
			}
			break;
		case GLUT_KEY_LEFT:  // 向左按键移动方块
			movetile(vec2(-1, 0));
			break;
		case GLUT_KEY_RIGHT: // 向右按键移动方块
			movetile(vec2(1, 0));
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// 键盘响应时间中的普通按键响应

void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // ESC键 和 'q' 键退出游戏
			exit(EXIT_SUCCESS);
			break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' 键重启游戏
			restart();
			break;
	}
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

void idle(void)
{
	glutPostRedisplay();
}

// 此函数实现每隔500毫秒方块自动下落一格
void TimeFunc(int val)
{
	if (!gameover) {
		// 下落一格的时候，需要检测是否到底，到底的话是否消除，消除便计算分数
		if (!movetile(vec2(0, -1)))
		{
			settile();
			checkfullrow(0);
			score += 5 * clean_up_row * (clean_up_row + 1);
			sprintf(score_out, "%d", score);
			glutSwapBuffers();
			clean_up_row = 0;
			newtile();
		}
	}
	// 如果分数大于100的话，增加方块下落速度
	// 以此提升游戏难度
	if (score >= 100) {
		glutTimerFunc(300, TimeFunc, 1);
	}
	else {
		// 在结尾重复调用计时器函数
		glutTimerFunc(500, TimeFunc, 1);

	}
}

//////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178);
	glutCreateWindow("2015150081_欧培_期中大作业");
	glewInit();
	init();

	glutDisplayFunc(display);	// display
	glutReshapeFunc(reshape);	// 窗口大小变化
	glutSpecialFunc(special);	// 响应方向键盘
	glutKeyboardFunc(keyboard);	// 相应r和q
	glutIdleFunc(idle);			// 设置全局的回调函数

	glutTimerFunc(500, TimeFunc, 1);// 此处增加了调用计时器函数
	
	glutMainLoop();			// 进入GLUT事件处理循环

	return 0;
}