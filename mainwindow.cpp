#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QWidget"
#include <QMouseEvent>
#include <mainwindow.h>
#include <iostream>
#include <sstream>
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
#include <iostream>
#include <thread>
#include "mainwindow.h"
#include <filesystem>
#include <iostream>

MainWindow::~MainWindow() {
    delete ui;
}

using namespace std;

void getCombos();

void handleBallRemoval();

const int teamSize = 6;
const int characterCount = 5;
const int width2 = 280;
const int height2 = 450;
const int battle = 3;
const int stoneCount = 6;
const int ballBorder = 45.0*45.0/4;
int ballWidth, ballHeight;

string stoneName[stoneCount] = {"water_stone", "fire_stone", "earth_stone",
                                "light_stone", "dark_stone", "heart_stone"};
string charName[characterCount] = {"ID1","ID2","ID3","ID4","ID5"};
string enemiesName[] = {"96n","98n","100n","102n","104n","180n","267n"};

// utils
uint64_t getMillis() {
    using namespace chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
        .count();
}

string toString(int i) {
    stringstream ss;
    ss << i;
    string str = ss.str();
    return str;
}

// Enemy Difinition
struct ENEMY {
    string fileName;
    int attr;
    int atk;
    int cd;
    int hp;
    int special;
    int id;
    QLabel* cdLabel;
    QLabel* healthLabel;

    ENEMY(const string& name, int a, int at, int cd, int h, int s,int id)
        : fileName(name), attr(a), atk(at), cd(cd), hp(h), special(s), id(id) {}
};

struct BATTLE {
    vector<ENEMY*>* enemies;
    BATTLE(vector<ENEMY*>* enemies) : enemies(enemies) {}
};

//ShowEnemy
vector<BATTLE*>* battles = new vector<BATTLE*>();
QLabel* enemyLabel[7];
QPixmap* enemyPic[7];

// Initializatios
QLineEdit* selectCharacter[teamSize];
QLineEdit* selectMission;
QPushButton* buttonConfirm;
QPushButton* buttonSettings;
QPushButton* buttonGoBack = NULL;
QPushButton* buttonSurrender = NULL;


//DFS
int board[5][6];
bool visited[5][6];

//STATIC
pair<int, int> boardCenter[5][6];

QLabel* boardLabel[5][6];

QPixmap* boardpic[stoneCount];
int characterType[6];
QLabel* characterLabel[6];
QPixmap* characterPic[characterCount];
QLabel* topLabel;
vector<vector<pair<int, int>>*>* combos = new vector<vector<pair<int, int>>*>();
int selectX, selectY;
bool movableState;
bool turnState;
bool selectState;
long lastMove = 0;
int origColor;
int lapx, lapy;
int damage[6];
int heal = 10000;
int level = 0;

void startTurn() {
    movableState = true;
    selectState = false;
    turnState = false;
}

std::atomic<bool> flag(false);
// Mouse Events

void MainWindow::mousePressEvent(QMouseEvent* event) {
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
            if ((dx - x) * (dx - x) + (dy - y) * (dy - y) <= ballBorder) {
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

int restraint(int a,int enemy){
    //water
    if(a == 1) {
        if(enemy == 2) {
            return 1;
        }
        if(enemy == 3) {
            return -1;
        }
    }
    if(a == 2){
        if(enemy == 3) {
            return 1;
        }
        if(enemy == 1) {
            return -1;
        }
    }
    if(a == 3){
        if(enemy == 1) {
            return 1;
        }
        if(enemy == 2) {
            return -1;
        }
    }
    if(a == 4) {
        if(enemy == 5) {
            return 1;
        }
    }
    if(a == 5) {
        if(enemy == 4){
            return 1;
        }
    }
    return 0;
}
void MainWindow::displayBoard() {
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            QPixmap* img = boardpic[board[i][j]];
            boardLabel[i][j]->setPixmap(*img);
            ballHeight = img->height();
            ballWidth = img->width();
            int side = (width2 - ballWidth * teamSize) / (teamSize + 1);
            int finalWidth = side * (j + 1) + ballWidth * j;
            int finalHeight = height2 - (5 - i) * (side + ballHeight);
            boardLabel[i][j]->setGeometry(finalWidth, finalHeight, ballWidth,
                                          ballHeight);
            boardLabel[i][j]->show();
            boardCenter[i][j] = {finalWidth + ballWidth / 2,
                                 finalHeight + ballHeight / 2};
        }
    }
}

void MainWindow::destory(){
    for(int i = 0;i<6;i++) {
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
            vector<pair<int, int>>* v = combos->at(i);
            for (pair<int, int> p : *v) {
                boardLabel[p.first][p.second]->hide();
                damage[board[p.first][p.second]]++;
                board[p.first][p.second] = -1;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
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
    damage[5]*=5;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    //CALCULATE DAMAGE
    /*
    for(int i = 0;i<6;i++) {
        string s = selectCharacter[i]->text().toStdString();
        if(s.length() != 0) {
            int type = s[0] - '0' - 1;

            vector<ENEMY*>* enemies = battles->at(level)->enemies;
            int size = enemies->size();

            int restraintTarget = -1, normalTarget = -1, badTarget = -1;
            for(int i = 0;i<size;i++) {
                ENEMY* enemy = enemies->at(i);
                if(enemy->hp <= 0)
                    continue;
                int result = restraint(type,enemy->attr);
                if(result == 1 && restraintTarget != -1) {
                    restraintTarget = i;
                }
                if(result == 0 && normalTarget != -1) {
                    normalTarget = i;
                }
                if(result == 0 && badTarget != -1) {
                    badTarget = i;
                }
            }
            int finalTarget = restraintTarget != -1 ? restraintTarget : (normalTarget != -1 ? normalTarget : badTarget);
        }
    }
    */
    startTurn();
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
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

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    int x = event->pos().x();
    int y = event->pos().y();
    if (!movableState) {
        return;
    }
    if (!selectState) {
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
            if ((dx - x) * (dx - x) + (dy - y) * (dy - y) <= ballBorder) {
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
        while (!out(x2, y2) && board[x2][y2] == orig) {
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

void MainWindow::updateCD(){
    vector<ENEMY*>* enemies = battles->at(level)->enemies;
    int size = enemies->size();
    for(int i = 0;i<size;i++) {
        ENEMY* enemy = enemies->at(i);
        enemy->cdLabel->setText(QString::fromStdString(toString(enemy->cd)));
    }
}
void MainWindow::showEnemy() {
    battles->clear();
    vector<ENEMY*>* x = new vector<ENEMY*>();
    x->push_back(new ENEMY("96n", 1, 200, 3, 100, 0,0));
    x->push_back(new ENEMY("98n", 2, 200, 3, 100, 0,1));
    x->push_back(new ENEMY("100n", 3, 200, 3, 100, 0,2));
    battles->push_back(new BATTLE(x));
    x = new vector<ENEMY*>();
    x->push_back(new ENEMY("102n", 4, 200, 3, 100, 0,3));
    x->push_back(new ENEMY("267n", 3, 200, 3, 300, 1,6));
    x->push_back(new ENEMY("104n", 5, 200, 3, 100, 0,4));
    battles->push_back(new BATTLE(x));
    x = new vector<ENEMY*>();
    x->push_back(new ENEMY("180n", 2, 400, 5, 700, 2,5));
    battles->push_back(new BATTLE(x));

    for(int i = 0;i<7;i++) {
        enemyLabel[i] = new QLabel(this);
        enemyPic[i] = new QPixmap(("../../resources/enemy/" + enemiesName[i] + ".png").c_str());
        *enemyPic[i] = enemyPic[i]->scaled(
            enemyPic[i]->width() / 2, enemyPic[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
        enemyLabel[i]->setPixmap(*enemyPic[i]);
        enemyLabel[i]->hide();
    }

    vector<ENEMY*>* enemies = battles->at(level)->enemies;
    int size = enemies->size();
    for(int i = 0;i<size;i++) {
        ENEMY* enemy = enemies->at(i);
        int w = enemyPic[enemy->id]->width();
        int h = enemyPic[enemy->id]->height();
        int side = (width2 - w * size) / (size + 1);
        int finalWidth = side * (i + 1) + w * i;
        enemyLabel[enemy->id]->setGeometry(finalWidth, 80, w,h);
        enemyLabel[enemy->id]->show();
        //cd
        enemy->cdLabel = new QLabel(this);
        enemy->cdLabel->setGeometry(finalWidth+10, 90, 15,15);
        enemy->cdLabel->setStyleSheet("QLabel { color : red; }");
        enemy->cdLabel->show();
    }
}
// Events
void MainWindow::handleStart() {
    level = 0;
    for (int i = 0; i < 6; i++) {
        selectCharacter[i]->hide();
    }
    buttonConfirm->hide();
    selectMission->hide();
    buttonSettings->show();

    //SET BOARD
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
            int side = (width2 - ballWidth * teamSize) / (teamSize + 1);
            int finalWidth = side * (i + 1) + ballWidth * i;
            int finalHeight = height2 - (6) * (side + ballHeight);
            characterLabel[i]->setGeometry(finalWidth, finalHeight, ballWidth,
                                           ballHeight);
            characterLabel[i]->show();
        }
    }
    displayBoard();
    startTurn();
    showEnemy();
    updateCD();
}

void MainWindow::handleGoButton() {
    if (selectCharacter[0]->text().toStdString().length() != 1) {
        return;
    }
    if (selectCharacter[teamSize - 1]->text().toStdString().length() != 1) {
        return;
    }
    handleStart();
}

void MainWindow::openStartPage(){
    if(buttonSurrender != NULL)
        buttonSurrender->hide();
    if(buttonGoBack != NULL)
        buttonGoBack->hide();
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 6; j++) {
            boardLabel[i][j]->hide();
        }
    }
    for (int j = 0; j < 6; j++) {
        characterLabel[j]->hide();
    }
    if(battles->size() != 0) {
        vector<ENEMY*>* enemies = battles->at(level)->enemies;
        for(int i = 0;i<enemies->size();i++) {
            enemies->at(i)->cdLabel->hide();
            enemyLabel[enemies->at(i)->id]->hide();
        }
    }
    for (int i = 0; i < teamSize; i++) {
        selectCharacter[i]->show();
        selectCharacter[i]->setText("");
    }
    selectMission->show();
    selectMission->setText("");
    buttonConfirm->show();
    buttonSettings->hide();
    movableState = false;
    selectState = false;
    turnState = false;
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);


    srand(time(NULL));

    int W = 30;
    int H = 30;
    for (int i = 0; i < teamSize; i++) {
        selectCharacter[i] = new QLineEdit(this);
        selectCharacter[i]->setMaxLength(1);
        selectCharacter[i]->setValidator(new QRegularExpressionValidator(
            *new QRegularExpression("^[1-5]$"), selectCharacter[i]));
        int side = (width2 - H * teamSize) / (teamSize + 1);
        selectCharacter[i]->setGeometry(side * (i + 1) + W * i, 100, W, H);
    }
    W = 100;
    H = 30;
    selectMission = new QLineEdit(this);
    selectMission->setMaxLength(1);
    selectMission->setValidator(new QRegularExpressionValidator(
        *new QRegularExpression("^1$"), selectMission));
    selectMission->setGeometry((width2 - W) / 2, 200, W, H);

    W = 100;
    H = 50;
    buttonConfirm = new QPushButton("Go", this);
    buttonConfirm->setGeometry((width2 - W) / 2, 300, W, H);

    QObject::connect(buttonConfirm, &QPushButton::clicked,
                     [&]() { handleGoButton(); });

    // Settings
    W = 60;
    H = 30;
    buttonSettings = new QPushButton("Settings", this);
    buttonSettings->setGeometry((width2 - W), 0, W, H);
    buttonSettings->hide();

    QObject::connect(buttonSettings, &QPushButton::clicked, [&]() {
        if(!movableState) {
            return;
        }

        // Resume
        W = 100;
        H = 30;
        buttonGoBack = new QPushButton("Resume", this);
        buttonGoBack->setGeometry((width2 - W) / 2, 100, W, H);

        QObject::connect(buttonGoBack, &QPushButton::clicked, [&]() {
            buttonGoBack->hide();
            buttonSurrender->hide();
        });

        // Surrender
        W = 100;
        H = 30;
        buttonSurrender = new QPushButton("Surrender", this);
        buttonSurrender->setGeometry((width2 - W) / 2, 200, W, H);

        QObject::connect(buttonSurrender, &QPushButton::clicked, [&]() {
            openStartPage();
        });

        buttonGoBack->show();
        buttonSurrender->show();
    });

    // Board

        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 6; j++) {
                boardLabel[i][j] = new QLabel(this);
            }
        }
    topLabel = new QLabel(this);
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
        characterLabel[j] = new QLabel(this);
    }
    for (int i = 0; i < 5; i++) {
        characterPic[i] = new QPixmap(("../../resources/character/" + charName[i] + ".png").c_str());
        *characterPic[i] = characterPic[i]->scaled(
            characterPic[i]->width() / 2, characterPic[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
    }

    openStartPage();

    QThread* t = QThread::create([this] {
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
}
