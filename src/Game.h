#pragma once
class Entity
{
public:
	float posx, posy, vx, vy; //实体坐标和速度 
};

class Block
{
public:
	int X1, X2, Y1, Y2;
};

class Wall : public Block
{
private:
public:
	int material;

	Wall();

	Wall(int X1, int Y1, int X2, int Y2, int material);

	void draw();
};

class Room
{
private:
public:

	int id;
	int built, wallNums;
	int go[5];
	vector<Wall> walls;

	Room(int id);

	void initRoom();

	void updateRoom();

	int isBuild();
};

class Map
{ //地图类
private:
public:
	int N; //房间个数
	vector<Room> rooms;

	void linkRoom();

	Map();

	Room getRoom(int n);

	vector<Room> getRooms();
};

class Creature : public Entity
{
public:
	float height, width;	//生物的高度和宽度
	int dirction, health, healthMax; //方向，血量,血量上限
	int inRoom;	//所在的房间
	int lastShootTime; //上次发射子弹的时间
	int shootCd; //发射CD
	int isRemoteAttack; //是否可以远程攻击(发射子弹)
	int bulletNums; // 单次发射的子弹数量
	int shootPower; //射出子弹的威力

	int changeX(int n);

	int changeY(int n);

	int isPassible();

	void Shoot(int X1, int Y1, int X2, int Y2, int from, int power);
};

class Bullet : public Entity
{
private:
public:
	IMAGE img;
	int radius, status, lifetime, power; //子弹的各属性 位置 速度 半径 是否还存活 剩余存活时间 子弹的威力
	int from;//子弹是谁打出来的 1 主角 2 怪物
	Bullet(float posx, float posy, float vx, float vy, int radius, int status, int lifetime, int from, int power);

	void draw();

	int getStatus();

	void update();

	int isCollideRocket(Creature creature);
};

class Player : public Creature
{
private:
public:
	float vy;//角色坐标的速度a
	int status; //角色状态 站立 - 1，跑动 - 2 举枪30-60度 - 3 举枪 60-90度 - 4 压枪 - 5
	void init(float height, float width, int posx, int posy, int healthMax, int health, int shootCd, int isRemoteAttack, int bulletNums, int shootPower);

	void draw();

	void update();

	void drawHeart();
};

class Enemy : public Creature//怪物的类
{
public:
	int kind, onFloor, fly;
	Enemy(float posx, float posy, int kind, int inRoom);

	void draw();

	void update();

	void updateVelforTarge();
};