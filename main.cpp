#include <bits/stdc++.h>
#include <mainwindow.h>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QThread>
#include <QTimer>
#include <filesystem>
#include <iostream>
#include <thread>

#include "mainwindow.h"

using namespace std;

void getCombos();

void handleBallRemoval();

const int teamSize = 6;
const int characterCount = 5;
const int width = 280;
const int height = 450;
const int battle = 3;
const int stoneCount = 6;
int ballWidth, ballHeight;

// utils
uint64_t getMillis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
        .count();
}

// Enemy Difinition
struct ENEMY {
    std::string fileName;
    int attr;
    int atk;
    int cp;
    int hp;
    int special;

    ENEMY(const std::string& name, int a, int at, int c, int h, int s)
        : fileName(name), attr(a), atk(at), cp(c), hp(h), special(s) {}
};

struct BATTLE {
    std::vector<ENEMY*>* enemies;
    BATTLE(vector<ENEMY*>* enemies) : enemies(enemies) {}
};

vector<BATTLE*>* battles = new vector<BATTLE*>();

void initEnemy() {
    vector<ENEMY*>* x = new vector<ENEMY*>();
    x->push_back(new ENEMY("96n", 1, 200, 3, 100, 0));
    x->push_back(new ENEMY("98n", 2, 200, 3, 100, 0));
    x->push_back(new ENEMY("100n", 3, 200, 3, 100, 0));
    battles->push_back(new BATTLE(x));
    x = new vector<ENEMY*>();
    x->push_back(new ENEMY("102n", 4, 200, 3, 100, 0));
    x->push_back(new ENEMY("267n", 3, 200, 3, 300, 1));
    x->push_back(new ENEMY("104n", 5, 200, 3, 100, 0));
    battles->push_back(new BATTLE(x));
    x = new vector<ENEMY*>();
    x->push_back(new ENEMY("180n", 2, 400, 5, 700, 2));
    battles->push_back(new BATTLE(x));
}

// Initializatios
QLineEdit* selectCharacter[teamSize];
QLineEdit* selectMission;

QPushButton* buttonConfirm;
QPushButton* buttonSettings;
QPushButton* buttonGoBack;
QPushButton* buttonSurrender;
QHBoxLayout* layout = new QHBoxLayout;
// Board Manage
string stoneName[stoneCount] = {"water_stone", "fire_stone", "earth_stone",
                                "light_stone", "dark_stone", "heart_stone"};
string charName[characterCount] = {"ID1","ID2","ID3","ID4","ID5"};
int board[5][6];
bool visited[5][6];
QLabel* boardLabel[5][6];
pair<int, int> boardCenter[5][6];
QPixmap* boardpic[stoneCount];
QLabel* characterLabel[6];
QPixmap* characterPic[characterCount];
QLabel* topLabel;
vector<vector<pair<int, int>>*>* combos = new vector<vector<pair<int, int>>*>();
int selectX, selectY;
bool movableState = false;
bool turnState = false;
bool selectState = false;
long lastMove = 0;
int origColor;
int lapx, lapy;
int damage[6];
int heal = 10000;

std::atomic<bool> flag(false);
// Mouse Events

void handleMousePressed(QMouseEvent* event) {
    int x = event->pos().x();
    int y = event->pos().y();
    if (!movableState) {
        return;
    }
    if (selectState) {
        return;
    }
    bool f = false;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            int dx = boardCenter[i][j].first, dy = boardCenter[i][j].second;
            if ((dx - x) * (dx - x) + (dy - y) * (dy - y) <= 400) {
                selectX = i;
                selectY = j;
                f = true;
                break;
            }
        }
        if (f) break;
    }
    selectState = f;
    if (!f) {
        return;
    }
    origColor = board[selectX][selectY];
    boardLabel[selectX][selectY]->hide();
    QPixmap* img = boardpic[board[selectX][selectY]];
    topLabel->setPixmap(*img);
    topLabel->setGeometry(x - ballWidth / 2, y - ballHeight / 2, ballWidth,
                          ballHeight);
    topLabel->show();
}

void displayBoard() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            QPixmap* img = boardpic[board[i][j]];
            boardLabel[i][j]->setPixmap(*img);
            ballHeight = img->height();
            ballWidth = img->width();
            int side = (width - ballWidth * teamSize) / (teamSize + 1);
            int finalWidth = side * (j + 1) + ballWidth * j;
            int finalHeight = height - (5 - i) * (side + ballHeight);
            boardLabel[i][j]->setGeometry(finalWidth, finalHeight, ballWidth,
                                          ballHeight);
            boardLabel[i][j]->show();
            boardCenter[i][j] = {finalWidth + ballWidth / 2,
                                 finalHeight + ballHeight / 2};
        }
    }
}

void destory(){
    for(int i = 0;i<6;i++){
        damage[i] = 0;
    }
    int finalCombo = 0;
    while(true) {
        getCombos();
        int combo = combos->size();
        if(combo == 0) {
            break;
        }
        finalCombo+= combo;
        for (int i = 0; i < combo; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        vector<pair<int, int>>* v = combos->at(i);
        for (pair<int, int> p : *v) {
            boardLabel[p.first][p.second]->hide();
            damage[board[p.first][p.second]]++;
            board[p.first][p.second] = -1;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    for (int c = 0; c < 6; c++) {
        int index = 0;
        int newArr[5];
        for (int r = 0; r < 5; r++) {
            newArr[r] = -1;
        }
        for (int r = 4; r >= 0; r--) {
            if (board[r][c] != -1) {
                newArr[index++] = board[r][c];
            }
        }
        for (int r = 4; r >= 0; r--) {
            int idx = 4 - r;
            board[r][c] = newArr[idx] == -1 ? rand() % 6 : newArr[idx];
        }
    }
    displayBoard();
    }
    for(int i = 0;i<6;i++) {
        damage[i]*=finalCombo;
    }
}

void handleMouseReleased(QMouseEvent* event) {
    if(!movableState) {
        return;
    }
    if (selectState) {
        selectX = 0;
        selectY = 0;
        selectState = false;
    }
    if (turnState) {
        turnState = false;
        movableState = false;
        boardLabel[lapx][lapy]->setPixmap(*boardpic[origColor]);
        boardLabel[lapx][lapy]->show();
        topLabel->hide();
        flag.store(true);
    }
}

void handleMouseMoved(QMouseEvent* event) {
    int x = event->pos().x();
    int y = event->pos().y();
    if (!selectState) {
        return;
    }
    if (!movableState) {
        return;
    }
    if (getMillis() - lastMove > 50) {
        topLabel->setGeometry(x - ballWidth / 2, y - ballHeight / 2, ballWidth,
                              ballHeight);
        lastMove = getMillis();
    }
    int x2 = 0, y2 = 0;
    bool f = false;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            int dx = boardCenter[i][j].first, dy = boardCenter[i][j].second;
            if ((dx - x) * (dx - x) + (dy - y) * (dy - y) <= 400) {
                x2 = i;
                y2 = j;
                f = true;
                break;
            }
        }
        if (f) break;
    }
    if (!f) {
        return;
    }
    if (x2 != selectX || y2 != selectY) {
        boardLabel[selectX][selectY]->setPixmap(*boardpic[board[x2][y2]]);
        swap(board[selectX][selectY], board[x2][y2]);
        boardLabel[selectX][selectY]->show();
        selectX = x2;
        selectY = y2;
        lapx = x2;
        lapy = y2;
        boardLabel[x2][y2]->hide();
        turnState = true;
    }
}

// Calculate Combo
int vec[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
bool out(int i, int j) { return i >= 5 || i < 0 || j >= 6 || j < 0; }
void DFS(int i, int j, int orig) {
    for (int k = 0; k < 4; k++) {
        int count = 0;
        int x2 = i, y2 = j, vx = vec[k][0], vy = vec[k][1];
        while (!out(x2, y2) && !visited[x2][y2] && board[x2][y2] == orig) {
            count++;
            x2 += vx;
            y2 += vy;
        }
        if (count >= 3) {
            while (count--) {
                visited[i + vx * count][j + vy * count] = true;
            }
        }
    }
}
void DFS2(int i, int j, int orig, vector<pair<int, int>>* v) {
    visited[i][j] = false;
    v->push_back({i, j});
    for (int k = 0; k < 4; k++) {
        int x2 = i + vec[k][0], y2 = j + vec[k][1];
        if (!out(x2, y2) && visited[x2][y2] && board[x2][y2] == orig) {
            DFS2(x2, y2, orig, v);
        }
    }
}

void getCombos() {
    combos->clear();
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            visited[i][j] = false;
        }
    }
    for (int i = 4; i >= 0; i--) {
        for (int j = 0; j < 6; j++) {
            if (!visited[i][j]) {
                DFS(i, j, board[i][j]);
            }
        }
    }
    for (int i = 4; i >= 0; i--) {
        for (int j = 0; j < 6; j++) {
            if (visited[i][j]) {
                vector<pair<int, int>>* v = new vector<pair<int, int>>();
                DFS2(i, j, board[i][j], v);
                combos->push_back(v);
            }
        }
    }
}

// Events
void handleStart() {
    for (int i = 0; i < 6; i++) {
        selectCharacter[i]->hide();
    }
    buttonConfirm->hide();
    selectMission->hide();

    buttonSettings->show();

    while (true) {
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                board[i][j] = (rand() % 6);
            }
        }
        getCombos();
        if (combos->size() == 0) {
            break;
        }
    }

    for(int i = 0;i<6;i++) {
        string s = selectCharacter[i]->text().toStdString();
        if(s.length() != 0) {
            QPixmap* img = characterPic[s[0] - '0' - 1];
            characterLabel[i]->setPixmap(*img);
            ballHeight = img->height();
            ballWidth = img->width();
            int side = (width - ballWidth * teamSize) / (teamSize + 1);
            int finalWidth = side * (i + 1) + ballWidth * i;
            int finalHeight = height - (6) * (side + ballHeight);
            characterLabel[i]->setGeometry(finalWidth, finalHeight, ballWidth,
                                          ballHeight);
            characterLabel[i]->show();
        }
    }
    displayBoard();
    movableState = true;
}
void handleGoButton() {
    if (selectCharacter[0]->text().toStdString().length() != 1) {
        return;
    }
    if (selectCharacter[teamSize - 1]->text().toStdString().length() != 1) {
        return;
    }
    handleStart();
}

// MAIN
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.setFixedSize(width, height);
    srand(time(NULL));

    initEnemy();

    int W = 30;
    int H = 30;
    for (int i = 0; i < teamSize; i++) {
        selectCharacter[i] = new QLineEdit(&w);
        selectCharacter[i]->setMaxLength(1);
        selectCharacter[i]->setValidator(new QRegularExpressionValidator(
            *new QRegularExpression("^[1-5]$"), selectCharacter[i]));
        int side = (width - H * teamSize) / (teamSize + 1);
        selectCharacter[i]->setGeometry(side * (i + 1) + W * i, 100, W, H);
    }
    W = 100;
    H = 30;
    selectMission = new QLineEdit(&w);
    selectMission->setMaxLength(1);
    selectMission->setValidator(new QRegularExpressionValidator(
        *new QRegularExpression("^1$"), selectMission));
    selectMission->setGeometry((width - W) / 2, 200, W, H);

    W = 100;
    H = 50;
    buttonConfirm = new QPushButton("Go", &w);
    buttonConfirm->setGeometry((width - W) / 2, 300, W, H);

    QObject::connect(buttonConfirm, &QPushButton::clicked,
                     [&]() { handleGoButton(); });

    // Settings
    W = 60;
    H = 30;
    buttonSettings = new QPushButton("Settings", &w);
    buttonSettings->setGeometry((width - W), 0, W, H);
    buttonSettings->hide();

    QObject::connect(buttonSettings, &QPushButton::clicked, [&]() {
        buttonGoBack->show();
        buttonSurrender->show();
    });

    // Resume
    W = 100;
    H = 30;
    buttonGoBack = new QPushButton("Resume", &w);
    buttonGoBack->setGeometry((width - W) / 2, 100, W, H);
    buttonGoBack->hide();

    QObject::connect(buttonGoBack, &QPushButton::clicked, [&]() {
        buttonGoBack->hide();
        buttonSurrender->hide();
    });

    // Surrender
    W = 100;
    H = 30;
    buttonSurrender = new QPushButton("Surrender", &w);
    buttonSurrender->setGeometry((width - W) / 2, 200, W, H);
    buttonSurrender->hide();

    QObject::connect(buttonSurrender, &QPushButton::clicked, [&]() {
        //
    });

    QGridLayout* layout = new QGridLayout(&w);
    // Board
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            boardLabel[i][j] = new QLabel(&w);
        }
    }
    topLabel = new QLabel(&w);
    for (int i = 0; i < stoneCount; i++) {
        boardpic[i] = new QPixmap(("../../resources/runestone/" + stoneName[i] + ".png").c_str());
        *boardpic[i] = boardpic[i]->scaled(
            boardpic[i]->width() / 2, boardpic[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
    }
    //Char
        for (int j = 0; j < 6; j++) {
            characterLabel[j] = new QLabel(&w);
        }
        for (int i = 0; i < 5; i++) {
            characterPic[i] = new QPixmap(("../../resources/character/" + charName[i] + ".png").c_str());
            *characterPic[i] = characterPic[i]->scaled(
                characterPic[i]->width() / 2, characterPic[i]->height() / 2,
                Qt::KeepAspectRatio,      // Keep aspect ratio
                Qt::SmoothTransformation  // Smooth transformation for better quality
                );
        }

    QThread* t = QThread::create([] {
        uint64_t now = getMillis();
        while (true) {
            if (getMillis() - now > 1000) {
                if (flag.load()) {
                    destory();
                }
                flag.store(false);
                now = getMillis();
            }
        }
    });
    t->start();

    w.show();
    return a.exec();
}
