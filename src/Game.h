#pragma once
class Entity
{
public:
	float posx, posy, vx, vy; //ʵ��������ٶ� 
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
{ //��ͼ��
private:
public:
	int N; //�������
	vector<Room> rooms;

	void linkRoom();

	Map();

	Room getRoom(int n);

	vector<Room> getRooms();
};

class Creature : public Entity
{
public:
	float height, width;	//����ĸ߶ȺͿ��
	int dirction, health, healthMax; //����Ѫ��,Ѫ������
	int inRoom;	//���ڵķ���
	int lastShootTime; //�ϴη����ӵ���ʱ��
	int shootCd; //����CD
	int isRemoteAttack; //�Ƿ����Զ�̹���(�����ӵ�)
	int bulletNums; // ���η�����ӵ�����
	int shootPower; //����ӵ�������

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
	int radius, status, lifetime, power; //�ӵ��ĸ����� λ�� �ٶ� �뾶 �Ƿ񻹴�� ʣ����ʱ�� �ӵ�������
	int from;//�ӵ���˭������� 1 ���� 2 ����
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
	float vy;//��ɫ������ٶ�a
	int status; //��ɫ״̬ վ�� - 1���ܶ� - 2 ��ǹ30-60�� - 3 ��ǹ 60-90�� - 4 ѹǹ - 5
	void init(float height, float width, int posx, int posy, int healthMax, int health, int shootCd, int isRemoteAttack, int bulletNums, int shootPower);

	void draw();

	void update();

	void drawHeart();
};

class Enemy : public Creature//�������
{
public:
	int kind, onFloor, fly;
	Enemy(float posx, float posy, int kind, int inRoom);

	void draw();

	void update();

	void updateVelforTarge();
};