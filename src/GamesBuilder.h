#include<io.h>
#include<cstring>
#include<vector>
#include<string.h>
#include <string>
#include "EasyXPng.h"
#include <map>
#include<algorithm>
#include <sstream>
using namespace std;
#pragma once
#pragma warning(disable:4996)

map<string, vector<int>> imgList; //根据名字返回对应图片的下标队列
IMAGE imgs[2000]; //图片数组
int ant = 0; //记录图片数量

bool cmp_greater(string a, string b) {
    if (a.length() != b.length()) {
        return a.length() < b.length();
    }
    else {
        return a < b;
    }
}

bool cmp_less(string a, string b) {
    if (a.length() != b.length()) {
        return a.length() > b.length();
    }
    else {
        return a > b;
    }
}

void imgLoader(string path, string imgname, int width, int height) {
    vector<int> tmp; 
    tmp.push_back(ant); //将当前数组下标推入
    std::wstring wstr(path.begin(), path.end());
    _TCHAR* Tstr = (_TCHAR*)(&wstr[0]);
    loadimage(&imgs[ant], Tstr, width, height); //将图片读入当前下标的图片数组
    ant++;
    imgList.insert(pair<string, vector<int>>(imgname, tmp));
}

void getAllFiles(string path, vector<string>& files, string fileType)
{
    // 文件句柄
    intptr_t hFile = 0;
    // 文件信息
    struct _finddata_t fileinfo;

    string p;

    if ((hFile = _findfirst(p.assign(path).append("\\*" + fileType).c_str(), &fileinfo)) != -1) {
        do {
            // 保存文件的全路径
            files.push_back(p.assign(path).append("\\").append(fileinfo.name));

        } while (_findnext(hFile, &fileinfo) == 0); //寻找下一个，成功返回0，否则-1

        _findclose(hFile);
    }
}

void gifLoader(string filePath, string gifname, int width, int height, string cmp) {

    vector<string> files;
    vector<int> tmp;
    getAllFiles(filePath, files, ".png");
    if (cmp == "greater") {
        sort(files.begin(), files.end(), cmp_greater);
    }
    else {
        sort(files.begin(), files.end(), cmp_less);
    }
    int size = files.size();
    for (int i = 0; i < size; i++) {
        tmp.push_back(ant);
        string str = files[i].c_str();
        std::wstring wstr(str.begin(), str.end());
        _TCHAR* Tstr = (_TCHAR*)(&wstr[0]);
        loadimage(&imgs[ant], Tstr, width, height);
        ant++;
    }
    imgList.insert(pair<string, vector<int>>(gifname, tmp));
}

void initNutsCrabGif(int width, int height) {
    gifLoader(".\\img\\material\\monster\\NutsCrab\\run\\", "nutscrab.run", width, height, "greater");
}

void initNutsSnakeGif(int width, int height) {
    gifLoader(".\\img\\material\\monster\\NutsSnake\\fly\\", "nutssnake.fly", width, height, "greater");
}

void initNutsJellyFishGif(int width, int height) {
    gifLoader(".\\img\\material\\monster\\NutsJellyFish\\fly\\", "nutsjellyfish.fly", width, height, "greater");
}

void initDoorOpenGif(int width, int height) {
    gifLoader(".\\img\\material\\door\\", "door.gif.open", width, height, "less");
}

void initDoorCloseGif(int width, int height) {
    gifLoader(".\\img\\material\\door\\", "door.gif.close", width, height, "greater");
}

void initPlayerGif(int width, int height) {
    gifLoader(".\\img\\material\\hero\\default\\stay\\", "hero.stay", width, height, "greater");
    gifLoader(".\\img\\material\\hero\\default\\run\\", "hero.run", width, height, "greater");
    gifLoader(".\\img\\material\\hero\\default\\shoot\\up_30_60", "hero.shoot.up_30_60", width, height, "greater");
    gifLoader(".\\img\\material\\hero\\default\\shoot\\up_60_90", "hero.shoot.up_60_90", width, height, "greater");
    gifLoader(".\\img\\material\\hero\\default\\shoot\\down", "hero.shoot.down", width, height, "greater");
}

void initOpeningAnimationGif(int width, int height) {
    gifLoader(".\\img\\OpeningAnimation_1", "animation.1", width, height, "greater");
    gifLoader(".\\img\\OpeningAnimation_2", "animation.2", width, height, "greater");
    gifLoader(".\\img\\OpeningAnimation_3", "animation.3", width, height, "greater");
}

void beginLoader() { //预先加载加载界面
    gifLoader(".\\img\\Loading", "Loading", 350, 800, "greater");
}


void initBulletGif(int width, int height) {
    gifLoader(".\\img\\material\\bullet\\monster", "bullet.enemy", width, height, "greater");
    imgLoader(".\\img\\material\\bullet\\bullet.png", "bullet.hero", width, height);
}

void turnRound(IMAGE* img, int weigh, int height) //图片翻转函数
{
    DWORD* a = GetImageBuffer(img);
    for (int i = 0; i < height; i++)
    {
        DWORD p1 = weigh * i;
        DWORD p2 = weigh * (i + 1) - 1;
        while (p1 < p2)
        {
            DWORD t = a[p1];
            a[p1] = a[p2];
            a[p2] = t;
            p1++;
            p2--;
        }
    }
}

void videoCreater(string gifname, int x, int y, int width, int height, double speed) {
    vector<int> k = imgList.find(gifname)->second;
    int size = k.size();
    for (int i = 0; i < size; i++) {
        putimagePng(x, y, &imgs[k[i]]);
        FlushBatchDraw();
        Sleep(1 / speed);
    }
}

void gifCreater(string gifname, int x, int y, int tick, int width, int height, int direction, double speed) {
    vector<int> k = imgList.find(gifname)->second;
    int size = k.size();
    int t = tick % (size * int((1.0 / speed)));
    int n = k[t / int((1.0 / speed))];
    IMAGE tmp = imgs[n];
    if (direction) {
        turnRound(&tmp, width, height);
    }
    putimagePng(x, y, &tmp);
}

void loadDrops() {
    imgLoader(".\\img\\material\\heart\\full.png", "drops.healthfull", 30, 30); //1
    imgLoader(".\\img\\material\\heart\\half.png", "drops.healthhalf", 30, 30); //2
    imgLoader(".\\img\\material\\drops\\healthmax.png", "drops.healthmax", 30, 30); //3
    imgLoader(".\\img\\material\\drops\\numballistic.png", "drops.numballistic", 30, 30); //..
    imgLoader(".\\img\\material\\drops\\bulletlifetime.png", "drops.bulletlifetime", 30, 30);
    imgLoader(".\\img\\material\\drops\\shootcd.png", "drops.shootcd", 30, 30);
}


void initImg() {
    imgLoader(".\\img\\material\\box\\box.png", "box", 30, 30);
    gifCreater("Loading", 400, 0, 1, 300, 800, 0, 1);
    imgLoader(".\\img\\wall_1.png", "wall_1", 10, 10);
    imgLoader(".\\img\\wall_3.png", "wall_3", 45, 10);
    gifCreater("Loading", 400, 0, 2, 300, 800, 0, 1);
    imgLoader(".\\img\\material\\heart\\empty.png", "heart_empty", 45, 10);
    imgLoader(".\\img\\material\\heart\\half.png", "heart_half", 45, 10);
    imgLoader(".\\img\\material\\heart\\full.png", "heart_full", 45, 10);
    loadDrops();
    gifCreater("Loading", 400, 0, 3, 300, 800, 0, 1);
    imgLoader(".\\img\\background\\1.png", "background.1", 250, 800);
    imgLoader(".\\img\\material\\door\\door6.png", "door.close", 40, 180);
    gifCreater("Loading", 400, 0, 4, 300, 800, 0, 1);
    imgLoader(".\\img\\material\\door\\door1.png", "door.open", 40, 180);
    imgLoader(".\\img\\menu\\menu_1.png", "menu.1", 600, 800);
    gifCreater("Loading", 400, 0, 5, 300, 800, 0, 1);
    initNutsCrabGif(40, 40);
    initNutsSnakeGif(60, 60);
    initNutsJellyFishGif(70, 64);
    gifCreater("Loading", 400, 0, 6, 300, 800, 0, 1);
    initPlayerGif(64, 80);
    initDoorCloseGif(40, 180);
    initBulletGif(20, 20);
    gifCreater("Loading", 400, 0, 7, 300, 800, 0, 1);
    initDoorOpenGif(40, 180);
    gifCreater("Loading", 400, 0, 8, 300, 800, 0, 1);
    initOpeningAnimationGif(1200, 800);
    gifCreater("Loading", 400, 0, 9, 300, 800, 0, 1);
}

string sti(string k, int f) {
    stringstream ss;
    ss << k;
    ss << f;
    ss >> k;
    return k;
}

string getContent(string res) {
    res = res.substr(res.find("&"), res.end() - res.begin());
    res = res.substr(1, res.end() - res.begin());
    res = res.substr(0, res.find("&"));
    return res;
}

void imgBatchPrinter(int id, int X1, int Y1, int X2, int Y2, int width, int height) {
    for (int i = X1; i < X2; i += width) {
        for (int j = Y1; j < Y2; j += height)
        {
            if (id == 1) putimagePng(i, j, &imgs[imgList.find("wall_1")->second[0]]);
            if (id == 3) putimagePng(i, j, &imgs[imgList.find("wall_3")->second[0]]);
        }
    }
}

int collisionBetweenEntity(float X1, float Y1, float X2, float Y2, float X3, float Y3, float X4, float Y4) //判断实体和墙有没有相交
{
    
    float a, b, s;
    a = min(X2, X4) - max(X1, X3);
    a = max(0, a);
    b = min(Y2, Y4) - max(Y1, Y3);
    b = max(0, b);
    if (a * b > 0)return 1;
    else return 0;
}

int posTransformCoordinate(int opt, int pos, int width)
{
    if (opt == 1)
    {
        if (pos == 0 || pos == 1)return 0;
        else return 1200 - width - 30;
    }
    else
    {
        if (pos == 0 || pos == 2)return 200;
        else return 790;
    }
}