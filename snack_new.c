#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

//地图行数
#define ROW 23
//地图列数
#define COL 44
//蛇的最大长度
#define LEN 50

//定义各个物体类别的枚举
enum types{
  NONE,
  HEAD,
  BODY,
  FOOD,
  WALL
};

//定义移动方向枚举
enum directions{
  LEFT,
  RIGHT,
  TOP,
  BOTTOM
};
//定义移动偏移量数组
int offset[4][2] = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};

//定义关卡枚举
enum chapter{
  WIN,
  EASY,
  NORMAL,
  HARD
};

//定义地图内单个格子的结构体
struct grid{
  types type;
};
//定义蛇结构体
struct snack{
  int x, y; //坐标
};


//创建地图
struct grid map[ROW][COL];
//定义蛇头和蛇尾和移动方向
struct snack head, tail;
directions direction;
//定义分数
int score = 0;
//本局游戏是否结束,1为晋级，2为失败
int isFinish = 0;
//定义行走路线栈，便于跟踪尾巴的路线
struct snack route[LEN];
int pHead = 0;
int pTail = 0;

//以下内容需要在关卡设定的时候初始化
//当前关卡和下一级关卡
chapter nowChapter = EASY;
chapter nextChapter = EASY;
//定义蛇的速度，范围是1-10，越大越快
int speed = 4;
//定义每个食物可以得到的分数
int foodScore = 1;
//定义完成本关卡所需的分数，需要小于蛇的最大长度
int levelUpScore;

//绘图和光标的相关函数
void SetPos(COORD a);
void SetPos(int i, int j);
void HideCursor();
void drawRow(int y, int x1, int x2, char ch);
void drawRow(COORD a, COORD b, char ch);
void drawCol(int x, int y1, int y2, char ch);
void drawCol(COORD a, COORD b, char ch);
void drawFrame(COORD a, COORD  b, char ch);
void drawFrame(int x1, int y1, int x2, int y2, char ch);

//游戏相关
void initMap();
void showMap();
void showInfo();
void initScene();
void genFood();
void move();
void flushMap();
void showScore();
void levelUp();
void gameOver();

int main(){
  //隐藏光标
  HideCursor();

  //开始游戏循环
  while(1){
    system("cls");//清屏
    initScene();//初始化整个场景

    _getch();//等待玩家确定开始游戏
    genFood();//生成最初的食物
    while(!isFinish){
      move();
    }

    (isFinish == 1) ? levelUp() : gameOver();
  }
  return 0;
}

//下面这堆是从别的地方抄的，很好用就对了
void SetPos(COORD a)// 移动光标（隐） 
{
    HANDLE out=GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(out, a);
}
 
void SetPos(int i, int j)// 移动光标
{
    COORD pos={i, j};
    SetPos(pos);
}
 
void HideCursor()//隐藏光标
{
    CONSOLE_CURSOR_INFO cursor_info = {1, 0}; 
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
 
//把第y行，[x1, x2) 之间的坐标填充为 ch
void drawRow(int y, int x1, int x2, char ch)
{
	int i;
    SetPos(x1,y);
    for(i = 0; i <= (x2-x1); i++)
        printf("%c",ch);
}
 
//在a, b 纵坐标相同的前提下，把坐标 [a, b] 之间填充为 ch
void drawRow(COORD a, COORD b, char ch)
{
    if(a.Y == b.Y)
        drawRow(a.Y, a.X, b.X, ch);
    else
    {
        SetPos(0, 25);
        printf("error code 01：无法填充行，因为两个坐标的纵坐标(x)不相等");
        system("pause");
    }
}
 
//把第x列，[y1, y2] 之间的坐标填充为 ch
void drawCol(int x, int y1, int y2, char ch)
{
    int y=y1;
    while(y!=y2+1)
    {
        SetPos(x, y);
        printf("%c",ch);
        y++;
    }
}
 
//在a, b 横坐标相同的前提下，把坐标 [a, b] 之间填充为 ch
void drawCol(COORD a, COORD b, char ch)
{
    if(a.X == b.X)
        drawCol(a.X, a.Y, b.Y, ch);
    else
    {
        SetPos(0, 25);
        printf("error code 02：无法填充列，因为两个坐标的横坐标(y)不相等");
        system("pause");
    }
}

//左上角坐标、右下角坐标、用row填充行、用col填充列
void drawFrame(COORD a, COORD  b, char ch)
{
    drawRow(a.Y, a.X, b.X, ch);
    drawRow(b.Y, a.X, b.X, ch);
    drawCol(a.X, a.Y+1, b.Y-1, ch);
    drawCol(b.X, a.Y+1, b.Y-1, ch);
}
 
void drawFrame(int x1, int y1, int x2, int y2, char ch)
{
    COORD a={x1, y1};
    COORD b={x2, y2};
    drawFrame(a, b, ch);
}

//从这里开始都是自己的内容
//初始化地图，从下面的三个函数中选，此处可自定义
void initMap(){
  //重置地图
  void clearMap();
  clearMap();

  //声明关卡函数
  void easyMap();
  void normalMap();
  void hardMap();

  //判断当前关卡
  switch(nowChapter){
  case EASY:
    easyMap();
    break;
  case NORMAL:
    normalMap();
    break;
  case HARD:
    hardMap();
    break;
  }
}

void clearMap(){
  int i,j;
  for(i=0; i<ROW; i++)
    for(j=0; j<COL; j++)
      map[i][j].type = NONE;
}

void easyMap(){
  int i,j;

  //初始化墙的位置
  i = 0;
  for(j=0; j<COL; ++j) map[i][j].type = WALL;
  i = ROW-1;
  for(j=0; j<COL; ++j) map[i][j].type = WALL;
  j = 0;
  for(i=0; i<ROW; ++i) map[i][j].type = WALL;
  j = COL-1;
  for(i=0; i<ROW; ++i) map[i][j].type = WALL;

  //初始化蛇的位置
  head.x = tail.x = 20;
  head.y = tail.y = 10;
  tail.y++;
  map[head.x][head.y].type = HEAD;
  map[tail.x][tail.y].type = BODY;

  //初始化其他变量
  nextChapter = NORMAL;
  speed = 4;
  foodScore = 1;
  levelUpScore = 3;
}

void normalMap(){
  //自行添加代码
  int i,j;

  //初始化墙的位置
  i = 0;
  for(j=0; j<COL; ++j) map[i][j].type = WALL;
  i = ROW-1;
  for(j=0; j<COL; ++j) map[i][j].type = WALL;
  j = 0;
  for(i=0; i<ROW; ++i) map[i][j].type = WALL;
  j = COL-1;
  for(i=0; i<ROW; ++i) map[i][j].type = WALL;

  //初始化蛇的位置
  head.x = tail.x = 20;
  head.y = tail.y = 10;
  tail.y++;
  map[head.x][head.y].type = HEAD;
  map[tail.x][tail.y].type = BODY;

  //初始化其他变量
  nextChapter = HARD;
  speed = 6;
  foodScore = 2;
  levelUpScore = 6;
}

void hardMap(){
  //自行添加代码
  int i,j;

  //初始化墙的位置
  i = 0;
  for(j=0; j<COL; ++j) map[i][j].type = WALL;
  i = ROW-1;
  for(j=0; j<COL; ++j) map[i][j].type = WALL;
  j = 0;
  for(i=0; i<ROW; ++i) map[i][j].type = WALL;
  j = COL-1;
  for(i=0; i<ROW; ++i) map[i][j].type = WALL;

  //初始化蛇的位置
  head.x = tail.x = 20;
  head.y = tail.y = 10;
  tail.y++;
  map[head.x][head.y].type = HEAD;
  map[tail.x][tail.y].type = BODY;

  //初始化其他变量
  nextChapter = WIN;
  speed = 8;
  foodScore = 3;
  levelUpScore = 3;
}

//将地图和蛇显示出来
void showMap(){
  int i,j;
  for(i=0; i<ROW; i++)
    for(j=0; j<COL; j++){
      if(map[i][j].type == WALL){
        SetPos(j, i);
        putch('\04');
      }
      else if(map[i][j].type == HEAD || map[i][j].type == BODY){
        SetPos(j, i);
        putch('\02');
      }
      else if(map[i][j].type == FOOD){
        SetPos(j, i);
        putch('\03');
      }
      else{
        SetPos(j, i);
        putch(' ');
      }
    }
}

//显示帮助和说明
void showInfo()
{
	SetPos(45,2);
	printf("Now chapter: %d", (int)nowChapter);
	SetPos(45,5);
	printf("Use w,a,s,d to control your snack.");
	SetPos(45,6);
	printf("Eat food to be stronger.");
	SetPos(45,7);
	printf("Break wall? You die.");
	SetPos(45,15);
	printf("Any key to start!");
}

//初始化整个场景，调用了其他初始化函数
void initScene(){
  initMap();
  showMap();
  showInfo();

  score = 0;
  isFinish = 0;
  pHead = 0;
  pTail = 0;

  direction = LEFT;

  if(speed>10 || speed<1)
    speed = 4;
}

void genFood(){
  //此处可以增加是否地图已满的判定代码，以防止后面无限循环

  //随机生成食物位置
  int x,y;
  while(1)
  {
    x = rand()%23;
    y = rand()%44;
    if(map[x][y].type == NONE)
      {
        map[x][y].type = FOOD;
        break;
      }
  }

  //打印食物
	SetPos(y, x);
	putch('\03');
}

//显示成绩
void showScore(){
  SetPos(45,10);
	printf("Score: %d, target: %d", score, levelUpScore);
}

//蛇移动的核心代码
void move(){
  int curSpeed = 11-speed;
  struct snack next = head;

  while(curSpeed--){
    if(_kbhit()){ //如果检测到有键盘输入的话
      char c;
      c = _getch();

      switch(c){
      case 'w':
      case 'W':
        next.x += offset[(int)TOP][0];
        next.y += offset[(int)TOP][1];
        direction = TOP;
        break;
      case 's':
      case 'S':
        next.x += offset[(int)BOTTOM][0];
        next.y += offset[(int)BOTTOM][1];
        direction = BOTTOM;
        break;
      case 'a':
      case 'A':
        next.x += offset[(int)LEFT][0];
        next.y += offset[(int)LEFT][1];
        direction = LEFT;
        break;
      case 'd':
      case 'D':
        next.x += offset[(int)RIGHT][0];
        next.y += offset[(int)RIGHT][1];
        direction = RIGHT;
        break;
      }
    }
    Sleep(20);
  }

  //结算
  if(map[next.x][next.y].type == HEAD){
    next.x += offset[(int)direction][0];
    next.y += offset[(int)direction][1];
  }

  if(map[next.x][next.y].type == NONE){
    map[next.x][next.y].type = HEAD;
    map[head.x][head.y].type = BODY;
    map[tail.x][tail.y].type = NONE;

    route[pHead++] = head;
    tail = route[pTail++];
    head = next;
  }
  else if(map[next.x][next.y].type == BODY || map[next.x][next.y].type == WALL){
    isFinish = 2;
    return;
  }
  else if(map[next.x][next.y].type == FOOD){
    map[next.x][next.y].type = HEAD;
    map[head.x][head.y].type = BODY;

    route[pHead++] = head;
    head = next;

    score += foodScore;
    genFood();
  }

  pHead = (pHead==LEN ? 0 : pHead);
  pTail = (pTail==LEN ? 0 : pTail);

  flushMap();
  showScore();

  if(score >= levelUpScore){
    isFinish = 1;
    return;
  }
}


void flushMap(){
  showMap();
}

void levelUp(){
  //自行添加代码
  nowChapter = nextChapter;
  SetPos(45,10);
  if(nowChapter == WIN){
    printf("Congratulations, you Win!");
    nowChapter = EASY;
  }
  else
    printf("Congratulations, you Level Up!");

  _getch();
}

void gameOver(){
  //自行添加代码
  nowChapter = EASY;
  SetPos(45,11);
	printf("Oh, no, you DIE!");

  _getch();
}
