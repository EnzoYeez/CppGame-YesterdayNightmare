#include <conio.h>
#include <direct.h>
#include <graphics.h>
#include <io.h>
#include <windows.h>

#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#undef UNICODE
#undef _UNICODE

#include "EasyXPng.h"
#include "GamesBuilder.h"
#include "Timer.h"

#define HEIGHT 800
#define WIDTH 1200
#define DEBUG 0  // 是否开启控制台调试信息
#pragma warning(disable : 4996)
using namespace std;
#pragma comment(lib, "Winmm.lib")
#pragma once
class Entity {
public:
    float posx, posy, vx, vy;  // 实体坐标和速度
    float height, width;       // 实体的高度和宽度
    int health;                // 耐久
    int inRoom;                // 所在的房间

    float changeX(float n);

    float changeY(float n);

    int isPassible();
};

class Block {
public:
    int X1, X2, Y1, Y2;
};

class Box : public Entity {
public:
    int id;  // 从3开始

    Box(float posx, float posy, int width, int height, int health, int inRoom,
        int vx, int vy, int id);

    void draw();

    void update();
};

class Drop : public Entity {
public:
    int id;       // 掉落物的id
    int visible;  // 是否可见
    int material;

    Drop(float posx, float posy, int width, int height, int visible, int inRoom,
        int vx, int vy, int material, int id);

    void update();

    void draw();
};

class Wall : public Block {
private:
public:
    int material;

    Wall();

    Wall(int X1, int Y1, int X2, int Y2, int material);

    void draw();
};

class Room {
private:
public:
    int id;
    int built, wallNums;
    int go[5];
    vector<Wall> walls;
    vector<Box> boxs;
    vector<Drop> drops;
    int doorStatus;
    int isMakeDrops;  // 是否已经放置掉落物

    Room(int id);

    void initRoom();

    void updateRoom();

    int isBuild();

    int isClear();

    void drawPosRoom();

    void showDoor();

    void showClosingDoor(int status);

    void showOpeningDoor(int status);

    Box* getBox(int id);

    void makeDrops(int nums);
};

class Map {  // 地图类
private:
public:
    int N;  // 房间个数
    vector<Room> rooms;

    void linkRoom(int level);

    void initMap(int level);

    Room* getRoom(int n);

    vector<Room> getRooms();
};

class Creature : public Entity {
public:
    int numBallistic;                 // 弹道数
    int dirction, health, healthMax;  // 方向，血量,血量上限
    int lastShootTime;                // 上次发射子弹的时间
    int shootCd;                      // 发射CD
    int jumpCd;                       // 跳跃CD
    int jumpMax;                      // 最大跳跃次数
    int jumpLeft;                     // 当前剩余跳跃次数
    int onFloor;                      // 是否落地
    int isRemoteAttack;               // 是否可以远程攻击(怪物)
    int bulletNums;                   // 子弹数量
    int shootPower;                   // 射出子弹的威力
    int bulletlifetime;               // 子弹射程;
};

class Bullet : public Entity {
private:
public:
    IMAGE img;
    int radius, status, lifetime, power;  // 子弹的各属性 半径 位置 射程 威力
    int from;  // 子弹是谁打出来的 1 主角 2 怪物
    Bullet(float posx, float posy, float vx, float vy, int radius, int status,
        int lifetime, int from, int power);  //

    void draw();

    int getStatus();

    void update();

    int isCollideRocket(Creature creature);
};

class Player : public Creature {
private:
public:
    float vy;  // 角色坐标的速度a
    int status;  // 角色状态 站立，跑动 举枪30-60度 举枪60-90度 低头
    int level;  // 当前层数
    Player(int level);

    void init();

    void draw();

    void update();

    void drawHeart();

    int addHealth(int n);

    int minusShootCd(int n) {
        if (shootCd - n > 50) {
            shootCd -= n;
            return 0;
        }
        return 1;
    }
};

class Enemy : public Creature  // 怪物的类
{
public:
    int kind, fly;
    Enemy(float posx, float posy, int kind, int inRoom);

    void draw();

    void update();

    void updateVelforTarge();

    void Shoot(int X1, int Y1, int X2, int Y2, int from, int power);
};

// 抽象基类，用于处理攻击
class AttackHandler {
public:
    virtual void handleDamage(std::vector<Bullet>& bullets,
        std::vector<Enemy>& enemies, Entity& entity) = 0;
};

// 玩家攻击的具体实现
class PlayerAttackHandler : public AttackHandler {
public:
    void handleDamage(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies,
        Entity& entity) override {
        for (auto it1 = bullets.begin(); it1 != bullets.end();) {
            it1->update();  // 更新子弹位置

            for (auto it2 = enemies.begin(); it2 != enemies.end(); ++it2) {
                if (it1->from == 1 && it2->health > 0 &&
                    collisionBetweenEntity(it1->posx, it1->posy - 2 * it1->radius,
                        it1->posx + 2 * it1->radius, it1->posy,
                        it2->posx, it2->posy - it2->height,
                        it2->posx + it2->width, it2->posy)) {
                    --it2->health;    // 减少敌人生命值
                    it1->status = 0;  // 子弹失效
                    break;
                }
            }
            if (it1->getStatus() == 0)
                it1 = bullets.erase(it1);
            else
                ++it1;
        }
    }
};

// 怪物攻击的具体实现
class MonsterAttackHandler : public AttackHandler {
private:
    float lastTime;

public:
    MonsterAttackHandler() : lastTime(0) {}

    void handleDamage(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies,
        Entity& entity) override {
        float nowTime = clock();

        for (auto it1 = bullets.begin(); it1 != bullets.end();) {
            if (nowTime - lastTime > 1000 && it1->from == 2 &&
                collisionBetweenEntity(it1->posx, it1->posy - 2 * it1->radius,
                    it1->posx + 2 * it1->radius, it1->posy,
                    entity.posx, entity.posy - entity.height,
                    entity.posx + entity.width, entity.posy)) {
                lastTime = nowTime;
                entity.health--;  // 减少实体（英雄）的生命值
                it1->status = 0;  // 子弹失效
                break;
            }
            if (it1->getStatus() == 0)
                it1 = bullets.erase(it1);
            else
                ++it1;
        }

        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
            if (nowTime - lastTime > 1000 &&
                collisionBetweenEntity(entity.posx, entity.posy - entity.height,
                    entity.posx + entity.width, entity.posy,
                    it->posx, it->posy - it->height,
                    it->posx + it->width, it->posy)) {
                lastTime = nowTime;
                entity.health--;  // 减少实体（英雄）的生命值
                break;
            }
        }
    }
};

// 更新函数，使用多态处理攻击
void updateWithoutInput1() {
    int tick = 1;
    tick++;
    if (tick > 0x3f3f3f3f) tick = 1;
    Player hero(1);
    Map maps;
    vector<Bullet> bullets;
    vector<Enemy> monsters[233];  // 每个房间都有一个存怪物的vector
    hero.update();                // 更新英雄位置

    int nowRoom = hero.inRoom;
    Room* now = maps.getRoom(nowRoom);

    // 使用多态处理攻击
    std::unique_ptr<AttackHandler> attackHandler;

    // 根据实体类型来决定使用哪种攻击处理器
    if (dynamic_cast<Player*>(&hero)) {
        attackHandler = std::make_unique<PlayerAttackHandler>();
    }
    else {
        attackHandler = std::make_unique<MonsterAttackHandler>();
    }

    attackHandler->handleDamage(bullets, monsters[nowRoom], hero);
}

class playerData {
public:
    char name[100];
    int totilKilled;
    int maxOnceKilled;
    int level;
    float lastTime;
    string datetime;

    playerData();

    void save();

    void clear();
};
class configData {
public:
    configData();

    void save();

    int getEffects();

    int changeEffects();

private:
    int effects;  // 特效等级
};

Map maps;
const float g = 0.015;  // 重力加速度
const int WALL_SIZE = 10;
int tick = 1;
float posx_mouse, posy_mouse;  // 鼠标坐标
Player hero(1);
playerData pdata;
vector<Bullet> bullets;
vector<Enemy> monsters[233];  // 每个房间都有一个存怪物的vector
IMAGE heart_empty, heart_half, heart_full;  // 临时变量以后删 用来画血量

float beginTime = clock();
int onceKilled;
int restart = 0;  // 用于控制是否跳出游戏
configData pcfg;

//文件读写功能
configData::configData() {
    string folderPath = "./data";
   
    // 创建 data 文件夹
    struct stat info;
    if (stat(folderPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
#ifdef _WIN32
        _mkdir(folderPath.c_str());
#else
        mkdir(folderPath.c_str(), 0733);
#endif
    }

    ifstream infile("./data/config.gm");
    if (infile.is_open()) {
        infile >> effects;
        infile.close();
    }
    else {
        ofstream outfile("./data/config.gm");
        effects = 1;
        if (outfile.is_open()) {
            outfile << effects << endl;
            outfile.close();
        }
        else {
            cerr << "Unable to open file for writing" << std::endl;
        }
    }
}

void configData::save() {
    ofstream outfile("./data/config.gm");
    if (outfile.is_open()) {
        outfile << effects << std::endl;
        outfile.close();
    }
    else {
        std::cerr << "Unable to open file for writing" << std::endl;
    }
}

int configData::getEffects() { return effects; }

int configData::changeEffects() {
    effects = (effects == 1) ? 2 : 1;
    return 0;
}

playerData::playerData() {
    std::string folderPath = "./data";

    // 创建 data 文件夹
    struct stat info;
    if (stat(folderPath.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
#ifdef _WIN32
        _mkdir(folderPath.c_str());
#else
        mkdir(folderPath.c_str(), 0733);
#endif
    }

    std::ifstream infile("./data/data.gm");
    if (infile.is_open()) {
        infile >> name >> totilKilled >> maxOnceKilled >> level >> lastTime;
        infile.close();
    }
    else {
        time_t t;
        srand(static_cast<unsigned>(time(&t)));
        std::string k = "Player_" + std::to_string(rand() % 10000);
        std::ofstream outfile("./data/data.gm");
        if (outfile.is_open()) {
            outfile << k << "\n"
                << 0 << "\n"
                << 0 << "\n"
                << 0 << "\n"
                << 0.0f << std::endl;
            outfile.close();
        }
        else {
            std::cerr << "Unable to open file for writing" << std::endl;
        }
        std::strcpy(name, k.c_str());
        totilKilled = 0;
        maxOnceKilled = 0;
        level = 0;
        lastTime = 0.0f;
    }
}

void playerData::save() {
    std::ofstream outfile("./data/data.gm");
    if (outfile.is_open()) {
        outfile << name << "\n"
            << totilKilled << "\n"
            << maxOnceKilled << "\n"
            << level << "\n"
            << lastTime << std::endl;
        outfile.close();
    }
    else {
        std::cerr << "Unable to open file for writing" << std::endl;
    }
}

void playerData::clear() {
    std::strcpy(name, "");
    totilKilled = 0;
    maxOnceKilled = 0;
    level = 0;
    lastTime = 0.0f;
}

float Entity::changeX(float n)  // x坐标的变动 能动返回1 否则返回0
{
    posx += n;
    int k = isPassible();
    if (k != 1) {  // 判断是否合法
        posx -= n;
        return k;
    }
    return 1;
}

float Entity::changeY(float n)  // y坐标的变动 能动返回1 否则返回0
{
    posy += n;
    int k = isPassible();
    if (k != 1) {  // 判断是否合法
        posy -= n;
        return k;
    }
    return 1;
}

int Entity::isPassible()  // 生物位置变动后是否合法（不和墙相交就是合法）
{
    Room* k = maps.getRoom(inRoom);  // 获取房间
    for (auto it = k->walls.begin(); it != k->walls.end(); it++) {  // 迭代所有的墙
        if (collisionBetweenEntity(posx, posy - height, posx + width, posy, it->X1,
            it->Y1, it->X2, it->Y2))
            return 0;  // 有交界返回0
    }
    for (auto it = k->boxs.begin(); it != k->boxs.end(); it++) {  // 迭代所有的箱子
        if (&it->posx == &posx)
            continue;  // 如果posx的地址相同 -> 同一个箱子，跳过判断
        if (it->health <= 0) continue;  // 被打爆的箱子跳过判断
        if (collisionBetweenEntity(posx, posy - height, posx + width, posy,
            it->posx, it->posy - it->height,
            it->posx + it->width, it->posy))
            return it->id;
    }
    if (!k->isClear()) {  // 有怪的时候阻止走出房间
        if (posx < 0 || posx > WIDTH - 80) {
            return 0;  // 有交界返回0
        }
    }
    return 1;  // 可以通行返回1
}

Box::Box(float posx, float posy, int width, int height, int health, int inRoom,
    int vx, int vy, int id) {
    this->posx = posx;
    this->posy = posy;
    this->width = width;
    this->height = height;
    this->health = health;
    this->inRoom = inRoom;
    this->vx = vx;
    this->vy = vy;
    this->id = id;
}

void Box::draw() {
    gifCreater("box", posx, posy - height, tick, width, height, 0, 1);
}

void Box::update() {
    vy += g;
    if (changeY(vy) != 1) {
        vy = 0;
    }
}

Drop::Drop(float posx, float posy, int width, int height, int visible,
    int inRoom, int vx, int vy, int material, int id) {
    this->posx = posx;
    this->posy = posy;
    this->width = width;
    this->height = height;
    this->visible = visible;
    this->inRoom = inRoom;
    this->vx = vx;
    this->vy = vy;
    this->material = material;
    this->id = id;
}

void Drop::draw() {
    if (visible != 1) return;
    if (material == 1)
        gifCreater("drops.healthfull", posx, posy - height, tick, width, height, 0,
            1);
    if (material == 2)
        gifCreater("drops.healthhalf", posx, posy - height, tick, width, height, 0,
            1);
    if (material == 3)
        gifCreater("drops.healthmax", posx, posy - height, tick, width, height, 0,
            1);
    if (material == 4)
        gifCreater("drops.numballistic", posx, posy - height, tick, width, height,
            0, 1);
    if (material == 5)
        gifCreater("drops.bulletlifetime", posx, posy - height, tick, width, height,
            0, 1);
    if (material == 6)
        gifCreater("drops.shootcd", posx, posy - height, tick, width, height, 0, 1);
}

void Drop::update() {
    vy += g;
    if (changeY(vy) != 1) {
        vy = 0;
    }
}

Wall::Wall(int X1, int Y1, int X2, int Y2, int material) {
    this->X1 = X1;
    this->X2 = X2;
    this->Y1 = Y1;
    this->Y2 = Y2;
    this->material = material;
}

void Wall::draw() {
    if (material == 1) imgBatchPrinter(1, X1, Y1, X2, Y2, 10, 10);
    if (material == 2) imgBatchPrinter(3, X1, Y1, X2, Y2, 45, 10);
    // 画墙，因为直接调用会访问冲突，所以开了个函数画
}

Room::Room(int id) {
    this->id = id;
    walls.reserve(32);  // 预分配内存，提速，防止溢出
    for (int i = 0; i < 4; ++i) go[i] = -1;
    doorStatus = 1;
}

void Room::initRoom()  // 一个房间里面造墙
{
    int d = 3;
    walls.push_back(Wall(0, 0, 1200, 10, 1));                       // 天花板
    walls.push_back(Wall(0, 790, 1200, 800, 1));                    // 地板
    walls.push_back(Wall(0, 200, 40, 600, 1));                      // 左
    walls.push_back(Wall(1160, 200, 1200, 600, 1));                 // 右
    if (go[0] < 0) walls.push_back(Wall(0, 0, 40, 600, 1));         // 左上
    if (go[1] < 0) walls.push_back(Wall(0, 200, 40, 800, 1));       // 左下
    if (go[2] < 0) walls.push_back(Wall(1160, 0, 1200, 600, 1));    // 右上
    if (go[3] < 0) walls.push_back(Wall(1160, 200, 1200, 800, 1));  // 右下

    for (int i = 120; i < 600; i += (rand() % 50) + 100)  // 随机造一些墙
    {
        int t = (rand() % 4) + 1;
        while (t--) {
            int j = (rand() % 700) + 100;
            int l = 90 + (rand() % 5) * 45;
            if (500 < j && j < 700 && 350 < i && i < 450) {
                continue;
            }
            if (rand() % 3 == 2) {
                boxs.push_back(Box(j + (rand() % l), i - 10, 30, 30, 4, id, 0, 0, d));
                d++;
            }
            walls.push_back(Wall(j, i, j + l, i + 10, 2));
            j += (rand() % 100) + 100;
        }
    }
    built = 1;
    isMakeDrops = 0;
}
float BGX[2333], BGY[2333], BGR[2333];
void drawBackGround()  // 画背景
{
    static int finish = 0, numStar = 0;
    if (finish == 0) {
        numStar = 300;
        for (int i = 1; i <= numStar; ++i) {
            BGX[i] = rand() % 1200;
            BGY[i] = rand() % 800;
            BGR[i] = rand() % 2 + 1;
        }
        finish = 1;
    }
    for (int i = 1; i <= numStar; ++i) {
        // setfillcolor(RGB(0,0,0));
        setfillcolor(RGB(rand() % 255, rand() % 255, rand() % 255));
        solidcircle(BGX[i], BGY[i], 1);
    }
}

void Room::updateRoom() {  // 更新房间
    if (pcfg.getEffects() == 2) drawBackGround();
    drawPosRoom();
    for (auto it = walls.begin(); it != walls.end();
        it++) {  // 把房间里的墙全部画出来
        it->draw();
    }

    for (auto it = drops.begin(); it != drops.end(); it++) {
        it->update();
        it->draw();
    }

    if (!isClear()) {
        if (doorStatus < 120) {
            doorStatus++;
            showClosingDoor(doorStatus / 20);

        }
        else {
            showDoor();
        }
    }
    else {
        if (doorStatus >= 120 && doorStatus < 240) {
            showOpeningDoor(doorStatus / 20);
            doorStatus++;
        }
    }
    if (isClear() && isMakeDrops == 0) {
        if (rand() % 2 == 0) {
            makeDrops(1);
            makeDrops(1);
            makeDrops(1);
        }
        isMakeDrops = 1;
    }
}

int Room::isBuild() {  // 获取是否初始化完毕
    return built;
}

int Room::isClear() { return monsters[id].size() == 0; }

void Room::drawPosRoom() {
    LOGFONT f;
    TCHAR s[20];
    _stprintf(s, _T("%d"), id + 1);
    gettextstyle(&f);  // 获取当前字体设置
    f.lfHeight = 100;  // 设置字体高度
    _tcscpy(f.lfFaceName, _T("SansTaiNa"));
    /*settextcolor(HSVtoRGB(bk_color,0.8,0.8));*/
    settextstyle(&f);
    outtextxy(550, 350, s);
}

void Room::showClosingDoor(int status) {
    if (go[0] >= 0) gifCreater("door.gif.close", 1, 20, status, 40, 180, 0, 1);
    if (go[1] >= 0)
        gifCreater("door.gif.close", 1, HEIGHT - 190, status, 40, 180, 0, 1);
    if (go[2] >= 0)
        gifCreater("door.gif.close", WIDTH - 40, 20, status, 40, 180, 0, 1);
    if (go[3] >= 0)
        gifCreater("door.gif.close", WIDTH - 40, HEIGHT - 190, status, 40, 180, 0,
            1);
}

void Room::showOpeningDoor(int status) {
    if (go[0] >= 0) gifCreater("door.gif.open", 1, 20, status, 40, 180, 0, 1);
    if (go[1] >= 0)
        gifCreater("door.gif.open", 1, HEIGHT - 190, status, 40, 180, 0, 1);
    if (go[2] >= 0)
        gifCreater("door.gif.open", WIDTH - 40, 20, status, 40, 180, 0, 1);
    if (go[3] >= 0)
        gifCreater("door.gif.open", WIDTH - 40, HEIGHT - 190, status, 40, 180, 0,
            1);
}

void Room::showDoor() {
    if (go[0] >= 0) gifCreater("door.close", 1, 20, tick, 40, 180, 0, 1);
    if (go[1] >= 0)
        gifCreater("door.close", 1, HEIGHT - 190, tick, 40, 180, 0, 1);
    if (go[2] >= 0) gifCreater("door.close", WIDTH - 40, 20, tick, 40, 180, 0, 1);
    if (go[3] >= 0)
        gifCreater("door.close", WIDTH - 40, HEIGHT - 190, tick, 40, 180, 0, 1);
}

Box* Room::getBox(int id) {
    if (id < 3) return NULL;
    return &boxs[id - 3];
}

void Room::makeDrops(int nums) {
    for (int j = 1; j <= nums; ++j) {
        Drop now(rand() % (WIDTH - 100) + 50, rand() % HEIGHT, 30, 30, 1, id, 0, 0,
            1, j);
        Drop tmp(rand() % (WIDTH - 100) + 50, rand() % HEIGHT, 30, 30, 1, id, 0, 0,
            1, j);
        while (1) {
            Drop tmp(rand() % (WIDTH - 100) + 50, rand() % HEIGHT, 30, 30, 1, id, 0,
                0, rand() % 6 + 1, j);
            if (tmp.isPassible() == 1) {
                now = tmp;
                break;
            }
        }
        drops.push_back(now);
    }
}

void Map::linkRoom(int level)  // 连接房间
{
    rooms.reserve(32);
    N = rand() % 3 + 2 + level;
    Room r(0);
    rooms.push_back(r);
    for (int i = 1, to, ok; i < N; ++i)  // 生成一棵树
    {
        ok = 0;
        Room r(i);
        while (1) {
            to = rand() % i;
            for (int j = 0; j < 4; ++j)
                if (rooms[to].go[j] == -1) {
                    ok = 1;
                    rooms[to].go[j] = i;
                    r.go[3 - j] = to;
                    break;
                }
            if (ok) break;
        }
        rooms.push_back(r);
    }
    for (int i = 0; i < N; ++i) rooms[i].initRoom();
}

void Map::initMap(int level)  // 地图生成器
{
    time_t t;
    srand((unsigned)time(&t));
    rooms.clear();
    linkRoom(level);
}

Room* Map::getRoom(int n) {  // 根据id获取房间的实体
    if (n > N - 1 || n < 0) {
        return NULL;
    }
    else {
        return &rooms[n];
    }
}

vector<Room> Map::getRooms() {  // 返回所有的房间
    return rooms;
}

void Enemy::Shoot(int X1, int Y1, int X2, int Y2, int from, int power) {
    if (isRemoteAttack && clock() - lastShootTime > shootCd) {
        float tmpX, tmpY;
        if (kind == 2) {
            tmpX = X1 + width / 2;
            tmpY = Y1 - height / 2;
            // 计算子弹的各个属性
        }
        if (kind == 3) {
            tmpX = X1 + width / 2;
            tmpY = Y1 - height / 2;
            Y2 = tmpY + 30;
            X2 = tmpX;
        }
        float jiao;
        if (abs(X2 - tmpX) < 2) {
            if (Y2 > tmpY)
                jiao = -3.1415926 / 2;
            else
                jiao = 3.1415926 / 2;
        }
        else {
            if (X2 > tmpX)
                jiao = atan((tmpY - Y2) / (X2 - tmpX));
            else
                jiao = atan((tmpY - Y2) / (X2 - tmpX)) + 3.1415926;
        }
        float now = jiao - 3.1415926 / 6 + 3.1415926 / 3 / (numBallistic + 1);
        for (int i = 1; i <= numBallistic;
            ++i, now += 3.1415926 / 3 / (numBallistic + 1)) {
            Bullet k(tmpX, tmpY, 7.0 * cos(now) / 5, 7.0 * -sin(now) / 5, 10, 1,
                bulletlifetime, 2, shootPower);
            bullets.push_back(k);
        }
        lastShootTime = clock();
    }
}

Bullet::Bullet(float posx, float posy, float vx, float vy, int radius,
    int status, int lifetime, int from, int power) {
    this->posx = posx;
    this->posy = posy;
    this->vx = vx;
    this->vy = vy;
    this->radius = radius;
    this->status = status;
    this->lifetime = lifetime;
    this->from = from;
    this->power = power;
    loadimage(&img, _T(".\\img\\material\\bullet\\bullet.png"), 2 * radius,
        2 * radius);
}

void Bullet::draw() {
    if (posx < 0 || posx > WIDTH || posy < 0 || posy > HEIGHT ||
        lifetime <= 0)  // 如果子弹出界了就删去
    {
        status = 0;
    }
    if (status == 1)  // 否则画出来
    {
        if (from == 1) {
            gifCreater("bullet.hero", posx, posy - 2 * radius, tick, 2 * radius,
                2 * radius, 0, 0.1);
        }
        if (from == 2) {
            gifCreater("bullet.enemy", posx, posy - 2 * radius, tick, 2 * radius,
                2 * radius, 0, 0.1);
        }
        lifetime--;
    }
}

int Bullet::getStatus() { return status; }

void Bullet::update()  // 更新子弹坐标
{
    posx += vx;
    posy += vy;
}

int Bullet::isCollideRocket(Creature creature)  // 判断生物是否和子弹碰撞
{
    float distance_x = abs(creature.posx - posx);
    float distance_y = abs(creature.posy - posy);

    if (collisionBetweenEntity(posx, posy - 2 * radius, posx + 2 * radius, posy,
        creature.posx, creature.posy - creature.height,
        creature.posx + creature.width, creature.posy))
        return 1;
    else
        return 0;
}

Player::Player(int level) { this->level = level; }

void Player::init() {
    this->height = 80;
    this->width = 64;
    this->posx = WIDTH / 2 - hero.width / 2;
    this->posy = 770;
    this->healthMax = 6;
    this->health = 6;
    this->shootCd = 500;
    this->jumpMax = 2;
    this->isRemoteAttack = 1;
    this->bulletNums = 1;
    this->shootPower = 1;
    this->numBallistic = 1;  // 弹道数
    this->bulletlifetime = 100;
    status = 1;
    jumpLeft = jumpMax;
    vy = 0;
    jumpCd = 200;
    int level = 1;
}

void Player::draw()  // 根据状态返回对应的贴图
{
    if (status == 1)
        gifCreater("hero.stay", posx, posy - height, tick, width, height, dirction,
            0.02);
    if (status == 2)
        gifCreater("hero.run", posx, posy - height, tick, width, height, dirction,
            0.02);
    if (status == 3)
        gifCreater("hero.shoot.up_30_60", posx, posy - height, tick, width, height,
            dirction, 0.02);
    if (status == 4)
        gifCreater("hero.shoot.up_60_90", posx, posy - height, tick, width, height,
            dirction, 0.02);
    if (status == 5)
        gifCreater("hero.shoot.down", posx, posy - height, tick, width, height,
            dirction, 0.02);

    drawHeart();  // 画血量ui
}

int Player::addHealth(int n) {
    if (health + n > healthMax) {
        int k = health + n - healthMax;
        health = healthMax;
        return k;
    }
    else {
        health += n;
    }
    return 0;
}

void Player::update() {
    vy += g;
    if (changeY(vy) != 1) {
        if (vy > 0) {
            jumpLeft = jumpMax;
        }
        vy = 0;
    }

    if (posx + width - 30 < 0)  // 向左进入其他房间
    {
        int toRoom;
        if (posy <= 400)  // 左上角
        {
            toRoom = maps.getRoom(inRoom)->go[0];
        }
        else  // 左下角
        {
            toRoom = maps.getRoom(inRoom)->go[1];
        }
        for (int i = 0; i < 4; i++) {
            if (maps.getRoom(toRoom)->go[i] == inRoom)  // 找到传送过去是哪个门
            {
                posx = posTransformCoordinate(1, i, width);  // 位置转坐标
                posy = posTransformCoordinate(2, i, width);  // 位置转坐标
                inRoom = toRoom;
            }
        }
        bullets.clear();  // 进入其他房间就把子弹全清空
    }
    if (posx + width - 30 > WIDTH)  // 向右进入其他房间
    {
        int toRoom;
        if (posy <= 400)  // 右上角
        {
            toRoom = maps.getRoom(inRoom)->go[2];
        }
        else  // 右下角
        {
            toRoom = maps.getRoom(inRoom)->go[3];
        }
        for (int i = 0; i < 4; i++) {
            if (maps.getRoom(toRoom)->go[i] == inRoom) {  // 找到传送过去是哪个门
                posx = posTransformCoordinate(1, i, width);  // 位置转坐标
                posy = posTransformCoordinate(2, i, width);  // 位置转坐标
                inRoom = toRoom;
            }
        }
        bullets.clear();  // 进入其他房间就把子弹全清空
    }
    Room* k = maps.getRoom(inRoom);
    for (auto it = k->drops.begin(); it != k->drops.end();
        it++) {  // 迭代所有的掉落物
        if (&it->posx == &posx)
            continue;  // 如果posx的地址相同 -> 同一个掉落物，跳过判断
        if (it->visible <= 0) continue;  // 被打爆的箱子跳过判断
        if (collisionBetweenEntity(posx, posy - height, posx + width, posy,
            it->posx, it->posy - it->height,
            it->posx + it->width, it->posy)) {
            if (it->material == 1) {
                int k = addHealth(2);
                if (k == 0) {
                    it->visible = 0;
                }
                if (k == 1) {
                    it->material = 2;
                }
            }
            if (it->material == 2) {
                int k = addHealth(1);
                if (k == 0) {
                    it->visible = 0;
                }
            }
            if (it->material == 3) {
                healthMax += 2;
                it->visible = 0;
            }
            if (it->material == 4) {
                numBallistic += 1;
                it->visible = 0;
            }
            if (it->material == 5) {
                bulletlifetime += 50;
                it->visible = 0;
            }
            if (it->material == 6) {
                minusShootCd(50);
                shootCd -= 50;
                shootCd = max(shootCd, 100);
                it->visible = 0;
            }
        }
    }
}

void Player::drawHeart()  // 画角色血量
{
    int nowx = 10;
    for (int i = 1; i <= health / 2; ++i)
        putimagePng(nowx, 20, &heart_full), nowx += 50;
    if (health & 1) putimagePng(nowx, 20, &heart_half), nowx += 50;
    for (int i = 1; i <= (healthMax - health) / 2; ++i)
        putimagePng(nowx, 20, &heart_empty), nowx += 50;
}

Enemy::Enemy(float posx, float posy, int kind, int inRoom) {
    this->posx = posx;
    this->posy = posy;
    this->vx = 0;
    this->vy = 10;
    this->kind = kind;
    this->inRoom = inRoom;
    this->bulletlifetime = 20000;
    onFloor = 0;
    if (kind == 1)  // 每种怪物的属性
    {
        health = 2 + hero.level;
        width = 40;
        height = 40;
        fly = 0;
        shootCd = -1;
        isRemoteAttack = 0;
        bulletNums = 0;
    }
    if (kind == 2)  // 每种怪物的属性
    {
        health = 3 + hero.level;
        width = 60;
        height = 60;
        fly = 1;
        shootCd = 1000;
        isRemoteAttack = 1;
        numBallistic = 1 + (hero.level / 3);
        bulletNums = 1;
    }
    if (kind == 3)  // 每种怪物的属性
    {
        this->posy = (int)posy % 300 + 30;
        health = 3 + hero.level;
        width = 70;
        height = 64;
        fly = 1;
        shootCd = 800;
        isRemoteAttack = 1;
        bulletNums = 1;
        numBallistic = 1 + (hero.level / 3);
        vx = 0.5;
    }
}

void Enemy::draw() {
    if (kind == 1) {
        if (health > 0)
            gifCreater("nutscrab.run", posx, posy - 37, tick, width, height, dirction,
                0.02);
    }
    if (kind == 2) {
        if (health > 0) {
            gifCreater("nutssnake.fly", posx, posy - 37, tick, width, height,
                dirction, 0.02);
        }
    }
    if (kind == 3) {
        if (health > 0) {
            gifCreater("nutsjellyfish.fly", posx, posy - 37, tick, width, height, 0,
                0.02);
        }
    }
}

void Enemy::update() {
    // vx == 0 时不做修改
    if (vx > 0) {
        dirction = 0;
    }
    if (vx < 0) {
        dirction = 1;
    }
    if (fly) {
        if (kind == 2) {
            updateVelforTarge();
            posx += vx;
            posy += vy;
        }
        if (kind == 3) {
            ;
            posx += vx;
            if (posx < 40 || posx > WIDTH - width - 30) {
                vx = -vx;
            }
        }
    }
    else {
        vy = 5;
        if (!onFloor)  // 判断是否已经落地
        {
            if (changeY(vy) != 1) {
                onFloor = 1;
                vx = 0.5;
            }
        }
        else {
            if (changeY(vy) == 1) {
                changeY(-vy);
                vx = -vx;
            }
        }
        if (changeX(vx) != 1 || posx < 0 || posx > WIDTH) {
            vx = -vx;
        }
    }
    if (isRemoteAttack) {
        Shoot(posx, posy, hero.posx, hero.posy, 2, 1);
    }
}

void Enemy::updateVelforTarge()  // 让会飞的怪的速度瞄向角色
{
    if (hero.posx > posx)
        vx = 0.5;  // 目标在怪左边，怪x方向速度向右
    else if (hero.posx < posx)
        vx = -0.5;  // 目标在怪右边，怪x方向速度向左
    if (hero.posy > posy)
        vy = 0.5;  // 目标在怪下方，怪y方向速度向下
    else if (hero.posy < posy)
        vy = -0.5;  // 目标在怪上方，怪y方向速度向上
}

int sumMonster;
void makeMonster()  // 怪物生成器
{
    sumMonster = 0;
    for (int i = 1; i < maps.N; ++i) {
        int numMonster = rand() % 2 + hero.level;
        sumMonster += numMonster;  // 随着层数上升，难度加大
        for (int j = 1; j <= numMonster; ++j) {
            Enemy now(rand() % 400 + 400, rand() % HEIGHT, (rand() % 3) + 1, i);
            while (1) {
                Enemy tmp(rand() % 400 + 400, rand() % HEIGHT, (rand() % 3) + 1, i);
                if (tmp.isPassible() == 1) {
                    now = tmp;
                    break;
                }
            }
            monsters[i].push_back(now);
        }
    }
}

void Init()  // 预处理
{
    maps.initMap(hero.level);
    cleardevice();
    if (hero.level == 1) {
        beginTime = clock();
        onceKilled = 0;
        hero.init();  // 角色初始化大小和坐标和血量
        maps.initMap(1);
        bullets.clear();
    }
    else {
        hero.posx = WIDTH / 2 - hero.width / 2;
        hero.posy = 770;
    }
    hero.inRoom = 0;  // 初始房间是0号
    for (int i = 0; i < maps.N; i++) {
        monsters[i].clear();
    }
    makeMonster();  // 造怪物
}

void drawMonster()  // 画怪物
{
    int nowRoom = hero.inRoom;
    for (vector<Enemy>::iterator it = monsters[nowRoom].begin();
        it != monsters[nowRoom].end(); ++it)
        it->draw();
}

void drawBullet() {
    for (auto it = bullets.begin(); it != bullets.end(); it++) it->draw();
}

void drawDeath()  // 画死亡界面
{
    mciSendString(_T("close shootMusic"), NULL, 0, NULL);
    mciSendString(_T("close jumpMusic"), NULL, 0, NULL);

    mciSendString(_T("open .\\music\\death.mp3 alias deathMusic"), NULL, 0,
        NULL);  // 打开失败音效
    mciSendString(_T("play deathMusic"), NULL, 0, NULL);
    while (1) {
        BeginBatchDraw();
        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 0, 0));
        settextstyle(120, 0, _T("宋体"));
        outtextxy(400, 350, _T("游戏失败"));
        FlushBatchDraw();
        cin.clear();
        char k = 'f';
        k = getch();
        if (k == '\r') {
            restart = 1;
            break;
        }
    }
}
void drawWin()  // 画胜利界面
{
    mciSendString(_T("open .\\music\\win.mp3 alias winMusic"), NULL, 0,
        NULL);  // 打开失败音效
    mciSendString(_T("play winMusic"), NULL, 0, NULL);
    while (1) {
        mciSendString(_T("close shootMusic"), NULL, 0, NULL);
        BeginBatchDraw();
        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 0, 0));
        settextstyle(120, 0, _T("宋体"));
        outtextxy(350, 350, _T("恭喜通关本层！"));
        FlushBatchDraw();
        bullets.clear();
        Sleep(3000);
        if (kbhit()) break;
    }
    hero.level++;
    Init();
}

void drawBox() {
    Room* now = maps.getRoom(hero.inRoom);
    for (auto it = now->boxs.begin(); it != now->boxs.end(); it++) {
        if (it->health > 0) it->draw();
    }
}

void show()  // 画出所有部分
{
    cleardevice();
    BeginBatchDraw();
    maps.getRoom(hero.inRoom)->updateRoom();
    drawBox();
    drawMonster();  // 画怪物
    drawBullet();   // 画子弹
    hero.draw();    // 画角色，位于最上的图层
    FlushBatchDraw();
}
void updateWithoutInput() {
    tick++;
    if (tick > 0x3f3f3f3f) tick = 1;

    hero.update();  // 对于HERO坐标更新

    int nowRoom = hero.inRoom;  // 怪物坐标的更新
    Room* now = maps.getRoom(nowRoom);
    static float lastTime = 0;
    for (vector<Bullet>::iterator it1 = bullets.begin();
        it1 != bullets.end();)  // 子弹的更新
    {
        it1->update();  // 子弹坐标的更新
        for (vector<Enemy>::iterator it2 = monsters[nowRoom].begin();
            it2 != monsters[nowRoom].end(); ++it2)  // 子弹打到怪的更新
        {
            if (it1->from == 1 && it2->health > 0 &&
                collisionBetweenEntity(it1->posx, it1->posy - 2 * it1->radius,
                    it1->posx + 2 * it1->radius, it1->posy,
                    it2->posx, it2->posy - it2->height,
                    it2->posx + it2->width,
                    it2->posy))  // 如果子弹碰到了怪
            {
                --it2->health;

                it1->status = 0;

                break;
            }
        }
        for (auto it2 = now->boxs.begin(); it2 < now->boxs.end(); it2++) {
            if (it2->health > 0 &&
                collisionBetweenEntity(it1->posx, it1->posy - 2 * it1->radius,
                    it1->posx + 2 * it1->radius, it1->posy,
                    it2->posx, it2->posy - it2->height,
                    it2->posx + it2->width,
                    it2->posy))  // 如果子弹碰到了怪
            {
                --it2->health;

                it1->status = 0;

                break;
            }
        }
        float nowTime = clock();
        if (nowTime - lastTime > 1000 && it1->from == 2 &&
            collisionBetweenEntity(
                it1->posx, it1->posy - 2 * it1->radius, it1->posx + 2 * it1->radius,
                it1->posy, hero.posx, hero.posy - hero.height,
                hero.posx + hero.width, hero.posy))  // 如果子弹碰到了人
        {
            lastTime = nowTime;
            hero.health--;
            it1->status = 0;
            break;
        }
        if (it1->getStatus() == 0)
            it1 = bullets.erase(it1);
        else
            ++it1;
    }
    for (auto it = now->boxs.begin(); it != now->boxs.end(); it++) {
        it->update();
    }
    for (vector<Enemy>::iterator it = monsters[nowRoom].begin();
        it != monsters[nowRoom].end();)  // 怪物的更新
    {
        it->update();  // 怪物坐标的更新
        if (it->health <= 0) {
            it = monsters[nowRoom].erase(it);
            char k[10];
            onceKilled++;
            pdata.totilKilled++;
            pdata.save();
            --sumMonster;
            if (sumMonster == 0) {
                drawWin();
                return;
            }
        }
        else
            ++it;
    }
    for (vector<Enemy>::iterator it = monsters[nowRoom].begin();
        it != monsters[nowRoom].end(); ++it)  // 检测怪物是否碰到了人
    {
        float nowTime = clock();
        if (nowTime - lastTime > 1000 &&
            collisionBetweenEntity(hero.posx, hero.posy - hero.height,
                hero.posx + hero.width, hero.posy, it->posx,
                it->posy - it->height, it->posx + it->width,
                it->posy))  // 如果子弹碰到了怪
        {                                      // 怪物碰到了人
            lastTime = nowTime;
            --hero.health;
            break;
        }
        if (hero.health <= 0)  // 角色死亡
        {
            if (hero.level > pdata.level) {
                pdata.level = hero.level;
            }
            if (hero.level == pdata.level) {
                pdata.lastTime = max(pdata.lastTime, clock() - beginTime);
            }
            if (onceKilled > pdata.maxOnceKilled) {
                pdata.maxOnceKilled = onceKilled;
            }
            pdata.save();

            drawDeath();
            return;
        }
    }
}

void updateWithInput() {
    int flag = 1 /*判断是否有操作，以此来更新角色状态图片*/;
    MOUSEMSG m;
    static int shoot = 0;            // 判断是否正在开枪
    static float lastShootTime = 0;  // 上次开枪时间
    static float lastJumpTime = 0;   // 上次跳跃时间
    if (MouseHit())                  // 关于鼠标的操作
    {
        m = GetMouseMsg();
        posx_mouse = m.x;
        posy_mouse = m.y;  // 鼠标位置

        if (m.uMsg == WM_LBUTTONDOWN) {
            shoot = 1;  // 开枪了
          
        }
        if (m.uMsg == WM_LBUTTONUP) {
            /*mciSendString(_T("close shootMusic"), NULL, 0, NULL);//关闭开枪音效*/
            shoot = 0;  // 不开枪了
        }
        flag = 0;
    }

    if (shoot) {
        if (posx_mouse < hero.posx)
            hero.dirction = 1; /*枪口的朝向*/
        else
            hero.dirction = 0;
        if (abs(posx_mouse - hero.posx) < abs(posy_mouse - hero.posy))
            hero.status = 4;
        else
            hero.status = 3;
        if (posy_mouse - hero.posy > 0) hero.status = 5;
    }

    if (shoot &&
        clock() - lastShootTime >
        hero.shootCd)  // 按下了左键 发射子弹 调整这里的300就可以调整射速
    {
        float tmpX = hero.posx + hero.width / 2,
            tmpY = hero.posy - hero.height / 2;  // 计算子弹的各个属性
        float jiao;
        if (abs(posx_mouse - tmpX) < 2) {
            if (posy_mouse > tmpY)
                jiao = -3.1415926 / 2;
            else
                jiao = 3.1415926 / 2;
        }
        else {
            if (posx_mouse > tmpX)
                jiao = atan((tmpY - posy_mouse) /
                    (posx_mouse - tmpX));  
            else
                jiao = atan((tmpY - posy_mouse) / (posx_mouse - tmpX)) + 3.1415926;
        }
        float now = jiao - 3.1415926 / 6 + 3.1415926 / 3 / (hero.numBallistic + 1);
        for (int i = 1; i <= hero.numBallistic;
            ++i, now += 3.1415926 / 3 / (hero.numBallistic + 1)) {
            Bullet k(tmpX, tmpY, 7.0 * cos(now), 7.0 * -sin(now), 10, 1,
                hero.bulletlifetime, 1, hero.shootPower);
            bullets.push_back(k);
        }
        lastShootTime = clock();
    }

    if (GetKeyState(0x41) < 0)  // 按下了A
    {
        int k = hero.changeX(-1);
        if (k >= 3) {
            if (maps.getRoom(hero.inRoom)->getBox(k)->changeX(-1) == 1) {
                maps.getRoom(hero.inRoom)->getBox(k)->changeX(-1);
            }
        }
        if (!shoot) hero.status = 2;
        if (flag) hero.dirction = 1;
        flag = 0;
    }
    if (GetKeyState(0x44) < 0)  // 按下了D
    {
        int k = hero.changeX(1);
        if (k >= 3) {
            if (maps.getRoom(hero.inRoom)->getBox(k)->changeX(1) == 1) {
                maps.getRoom(hero.inRoom)->getBox(k)->changeX(1);
            }
        }
        if (!shoot) hero.status = 2;
        if (flag) hero.dirction = 0;
        flag = 0;
    }
    if (GetKeyState(0x41) < 0 || GetKeyState(0x44) < 0) {
        mciSendString(_T("open .\\music\\run.mp3 alias runMusic"), NULL, 0,
            NULL);  // 打开跑步音效
        mciSendString(_T("play runMusic"), NULL, 0, NULL);
    }
    else {
        mciSendString(_T("close runMusic"), NULL, 0, NULL);
    }
    if (GetKeyState(0x57) < 0 && hero.jumpLeft > 0 &&
        clock() - lastJumpTime > hero.jumpCd)  // 关闭跑步音效
    {
        hero.jumpLeft--;
        hero.vy = -3;
        lastJumpTime = clock();

        mciSendString(_T("close jumpMusic"), NULL, 0, NULL);
        mciSendString(_T("open .\\music\\jump.mp3 alias jumpMusic"), NULL, 0,
            NULL);  // 打开跳跃音效
        mciSendString(_T("play jumpMusic"), NULL, 0, NULL);
    }
    // perfect_wall(hero.inRoom);//完善墙
    if (flag && hero.status != 3 && hero.status != 4 && hero.status != 5)
        hero.status = 1;  // 没有操作，重置角色状态
}

void LoadResources() {
    IMAGE loading;  // 画“加载中”的画面
    loadimage(&loading, _T(".\\img\\loading.png"), 1200, 800);  // 画“加载中”的画面
    putimagePng(0, 0, &loading);  // 画“加载中”的画面
    loadimage(&heart_empty, _T(".\\img\\material\\heart\\empty.png"), 50,
        50);  // 临时变量以后删 用来画血量
    loadimage(&heart_half, _T(".\\img\\material\\heart\\half.png"), 50,
        50);  // 临时变量以后删 用来画血量
    loadimage(&heart_full, _T(".\\img\\material\\heart\\full.png"), 50,
        50);  // 临时变量以后删 用来画血量
    initImg();      // 记载图片
}

void showMenu_2() {
    int firstflash = 1;  // 第一次进入的时候要先显示了UI再读入，不然输入前会黑屏
    int select = 1;
    int isInput = 0;
    string name = "";
    cleardevice();
    BeginBatchDraw();
    while (1) {
        cin.clear();
        char k = 'f';
        string pri = "游戏ID:       ";
        if (firstflash == 1) {
            name = pdata.name;
            firstflash = 0;
        }
        else {
            k = getch();
        }
        if (k == 0x08) {
            if (isInput == 1 && !name.empty()) name.pop_back();
        }
        else {
            if (isInput == 1) {
                if (select == 1) {
                    if (k == 'a' || k == 'A' || k == 'd' || k == 'D') {
                        pcfg.changeEffects();
                    }
                }
                if (select == 2) {
                    if (('a' <= k && k <= 'z') || ('A' <= k && k <= 'Z') || k == '_' ||
                        k == '-') {
                        name += k;
                    }
                }
            }
            else {
                if (k == 'w' || k == 'W') {
                    if (select > 1) {
                        select--;
                    }
                }
                if (k == 's' || k == 'S') {
                    if (select < 3) {
                        select++;
                    }
                }
            }
        }
        if (k == '\r') {
            if (select == 1 || select == 2) {
                isInput = isInput == 1 ? 0 : 1;
            }
            if (select == 3) {
                pdata.clear();
                strcpy(pdata.name, name.c_str());
                pdata.save();
                pcfg.save();
                break;
            }
        }

        pri += name;

        gifCreater("menu.1", WIDTH / 2 - 300, 0, tick, 600, 800, 0, 1);

        setbkmode(TRANSPARENT);
        settextstyle(20, 0, _T("宋体"), 0, 0, 800, 0, 0, 0);
        TCHAR tszWord[1024];
        char szWord[100];
        strcpy(szWord, pri.c_str());
        MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
        if (select == 1) {
            if (isInput == 1) {
                settextcolor(RGB(255, 0, 0));
            }
            else {
                settextcolor(RGB(64, 207, 235));
            }
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        if (pcfg.getEffects() == 1) {
            outtextxy(510, 390, _T("特效等级:     ◀ 低 ▶"));
        }
        else if (pcfg.getEffects() == 2) {
            outtextxy(510, 390, _T("特效等级:     ◀ 高 ▶"));
        }

        if (select == 2) {
            if (isInput == 1) {
                settextcolor(RGB(255, 0, 0));
            }
            else {
                settextcolor(RGB(64, 207, 235));
            }
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(510, 420, tszWord);
        if (select == 3) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(580, 450, _T("确认"));
        FlushBatchDraw();
    }
}

void showMenu_3() {
    int firstflash = 1; 
    int select = 1;
    cleardevice();
    BeginBatchDraw();
    while (1) {
        cin.clear();
        if (firstflash == 1) {
            firstflash = 0;
        }
        else {
            char k = 'f';
            k = getch();
            if (k == '\r') {
                if (select == 1) {
                    break;
                }
            }
        }
        gifCreater("menu.1", WIDTH / 2 - 300, 0, tick, 600, 800, 0, 1);

        settextcolor(RGB(133, 96, 208));
        setbkmode(TRANSPARENT);
        settextstyle(20, 0, _T("宋体"), 0, 0, 800, 0, 0, 0);
        string t;
        TCHAR tszWord[1024];
        char szWord[100];
        t = sti("总击杀数:", pdata.totilKilled);
        strcpy(szWord, t.c_str());
        MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
        outtextxy(470, 370, tszWord);
        t = sti("最高单轮击杀:", pdata.maxOnceKilled);
        strcpy(szWord, t.c_str());
        MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
        outtextxy(470, 400, tszWord);
        t = sti("最高层数:", pdata.level);
        strcpy(szWord, t.c_str());
        MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
        outtextxy(470, 430, tszWord);
        t = sti("最短用时:", (int)pdata.lastTime / 1000);
        t += "s";
        strcpy(szWord, t.c_str());
        MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
        outtextxy(470, 460, tszWord);

        if (select == 1) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(580, 500, _T("确认"));
        FlushBatchDraw();
    }
}

void showMenu_4() {
    int firstflash = 1;  
    int select = 1;
    int from = 1;
    int isChange = 1;  
    int to = 15;
    playerData pds[20];
    cleardevice();
    BeginBatchDraw();
    while (1) {
        cin.clear();
        if (firstflash == 1) {
            firstflash = 0;
        }
        else {
            char k = 'f';
            k = getch();
            if (k == '\r') {
                if (select == 1) {
                    break;
                }
            }
            if (k == 'A' || k == 'a') {
                if (from - 15 > 0) {
                    from -= 15;
                    to -= 15;
                    isChange = 1;
                }
            }
            if (k == 'D' || k == 'd') {
                from += 15;
                to += 15;
                isChange = 1;
            }
        }

        cleardevice();
        gifCreater("menu.1", WIDTH / 2 - 300, 0, tick, 600, 800, 0, 1);

        settextcolor(RGB(133, 96, 208));
        setbkmode(TRANSPARENT);
        settextstyle(15, 0, _T("宋体"), 0, 0, 800, 0, 0, 0);
        string t;
        TCHAR tszWord[1024];
        char szWord[100];
        settextcolor(RGB(64, 207, 235));
        outtextxy(300, 320, _T("游戏ID"));
        outtextxy(420, 320, _T("总击杀数"));
        outtextxy(510, 320, _T("最高单轮击杀"));
        outtextxy(650, 320, _T("最高层数"));
        outtextxy(750, 320, _T("最短用时"));
        outtextxy(850, 320, _T("上次更新"));
        for (int i = 0; i <= to - from; i++) {
            strcpy(szWord, pds[i].name);
            MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
            outtextxy(300, 350 + (i * 20), tszWord);
            t = sti("", pds[i].totilKilled);
            strcpy(szWord, t.c_str());
            MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
            outtextxy(420, 350 + (i * 20), tszWord);
            t = sti("", pds[i].maxOnceKilled);
            strcpy(szWord, t.c_str());
            MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
            outtextxy(510, 350 + (i * 20), tszWord);
            t = sti("", pds[i].level);
            strcpy(szWord, t.c_str());
            MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
            outtextxy(650, 350 + (i * 20), tszWord);
            t = sti("", pds[i].lastTime);
            strcpy(szWord, t.c_str());
            MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
            outtextxy(750, 350 + (i * 20), tszWord);
            t = pds[i].datetime;
            strcpy(szWord, t.c_str());
            MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
            outtextxy(850, 350 + (i * 20), tszWord);
        }

        if (select == 1) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }

        t = "";
        t = sti(t, from);
        t += "-";
        t = sti(t, to);
        strcpy(szWord, t.c_str());
        MultiByteToWideChar(CP_ACP, 0, szWord, -1, tszWord, 1024);
        outtextxy(320, 770, tszWord);
        settextstyle(20, 0, _T("宋体"), 0, 0, 800, 0, 0, 0);
        outtextxy(850, 770, _T("确认"));
        FlushBatchDraw();
    }
}

void showMenu_1() {
    int firstflash = 1; 
    int select = 1;
    BeginBatchDraw();
    while (1) {
        Sleep(100);
        char k = 'f';
        if (firstflash == 1) {
            firstflash = 0;
        }
        else {
            k = getch();
        }
        if (k == 'w' || k == 'W') {
            if (select > 1) {
                select--;
            }
        }
        if (k == 's' || k == 'S') {
            if (select < 5) {
                select++;
            }
        }
        if (k == 0x0D) {
            if (select == 1) {
                break;
            }
            if (select == 2) {
                showMenu_2();
            }
            if (select == 3) {
                showMenu_3();
            }
            if (select == 4) {
                showMenu_4();
            }
            if (select == 5) {
                exit(0);
            }
        }
        setbkmode(TRANSPARENT);
        settextstyle(20, 0, _T("宋体"), 0, 0, 800, 0, 0, 0);
        cleardevice();
        gifCreater("menu.1", WIDTH / 2 - 300, 0, tick, 600, 800, 0, 1);

        if (select == 1) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(560, 400, _T("进入试炼"));
        if (select == 2) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(560, 430, _T("游戏设置"));
        if (select == 3) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(560, 460, _T("个人成就"));
        if (select == 4) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(560, 490, _T("游戏榜单"));
        if (select == 5) {
            settextcolor(RGB(64, 207, 235));
        }
        else {
            settextcolor(RGB(133, 96, 208));
        }
        outtextxy(560, 520, _T("退出游戏"));
        FlushBatchDraw();
    }
}



int main()
{
    beginLoader();
    initgraph(1200, 800, 1);
    gifCreater("Loading", 400, 0, 0, 600, 300, 0, 1);


    pdata.save(); //将程序中的内容同步到文件
    setbkcolor(RGB(0, 0, 0));
    LoadResources();

    double dur;
    clock_t start = clock(), end = clock();

    while (1)
    {
        showMenu_1();

        cleardevice();
        /*下面是开头动画*/
        BeginBatchDraw();
        videoCreater("animation.1", 0, 0, HEIGHT, WIDTH, 0.1);
        while (1)
        {
            if (GetKeyState(0x45) < 0)
            {
                Sleep(10);
                break;
            }
        }
        videoCreater("animation.2", 0, 0, HEIGHT, WIDTH, 0.1);
        while (1)
        {
            if (GetKeyState(0x45) < 0)
            {
                Sleep(10);
                break;
            }
        }
        videoCreater("animation.3", 0, 0, HEIGHT, WIDTH, 0.1);
        EndBatchDraw();

        /*上面是开头动画*/
        hero.level = 1;
        Init();
        mciSendString(_T("open .\\music\\bgm.mp3 alias bkmusic"), NULL, 0, NULL);//打开背景音乐
        mciSendString(_T("play bkmusic repeat"), NULL, 0, NULL);  // 循环播放
        while (1)
        {
            show();//画
            updateWithoutInput();//更新 与输入无关
            updateWithInput();//更新 与输入有关
            /*下面用来测帧率*/
            end = clock();
            dur = (double)(100);
            if (tick % 10 == 0) printf("FPS: %f\n", 1 / (dur / CLOCKS_PER_SEC));
            //k.Sleep(1);//根据机器性能自行调整
            start = clock();
            if (restart) break;
        }
        mciSendString(_T("close bkmusic"), NULL, 0, NULL);
        restart = 0;
        Sleep(200);
    }
    _getch();
    closegraph();
    return 0;
}
