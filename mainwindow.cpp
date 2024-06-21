#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QWidget"
#include <QMouseEvent>
#include <mainwindow.h>
#include <iostream>
#include <sstream>
#include <QElapsedTimer>
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
#include <iostream>
#include <QTime>
#include <queue>

MainWindow::~MainWindow() {
    delete ui;
}

using namespace std;

void getCombos();

void handleBallRemoval();
//CONFIG
int dmgMultiplier = 50;

//STATIC
const int teamSize = 6;
const int characterCount = 5;
const int width2 = 280;
const int height2 = 450;
const int battle = 3;
const int stoneCount = 6;
const int ballBorder = 45.0*45.0/4;
int ballWidth, ballHeight;

QLabel* HEALLABEL;
string resPath = "../../resources/";

string stoneName[stoneCount] = {"water_stone", "fire_stone", "earth_stone",
                                "light_stone", "dark_stone", "heart_stone"};
string color[] = {"blue", "red", "green",
                                "yellow", "purple"};
string charName[characterCount] = {"ID1","ID2","ID3","ID4","ID5"};
string enemiesName[] = {"96n","98n","100n","102n","104n","180n","267n"};

// utils
uint64_t getMillis() {
    using namespace chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
        .count();
}

void sleep(long ms) {
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<ms)
        QCoreApplication::processEvents();
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
    int defcd;
    int hp;
    int dead = 0;
    int special;
    int id;
    int x;
    int y;
    QLabel* cdLabel;
    QLabel* healthLabel;

    ENEMY(const string& name, int a, int at, int cd, int h, int s,int id)
        : fileName(name), attr(a), atk(at), cd(cd), hp(h), special(s), id(id) {
        defcd = cd;
    }
};

struct BATTLE {
    vector<ENEMY*>* enemies;
    BATTLE(vector<ENEMY*>* enemies) : enemies(enemies) {}
};

//ShowEnemy
vector<BATTLE*>* battles = new vector<BATTLE*>();
QLabel* enemyLabel[7];
QPixmap* enemyPic[7];

//queue<QLabel*> damageQueue = new queue<QLabel*>();
// Initializatios
QLineEdit* selectCharacter[teamSize];
QLineEdit* selectMission;
QPushButton* buttonGoBackToMenu = NULL;
QPushButton* buttonConfirm;
QPushButton* buttonSettings;
QPushButton* buttonGoBack = NULL;
QPushButton* buttonSurrender = NULL;

//DFS
int board[5][6];
bool visited[5][6];
bool boardWeather[5][6];
bool boardFire[5][6];
deque<pair<int,int>>* fireQueue = new deque<pair<int,int>>();

//STATIC
pair<int, int> boardCenter[5][6];

QLabel* boardLabel[5][6];

QPixmap* boardpic[stoneCount];
QPixmap* boardpicWeather[stoneCount];
QPixmap* boardpicFire[stoneCount];
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
int damageALL[6];
int heal = 10000;
int maxheal = heal;
int level = 0;
bool fireDamage = false;
bool MainWindow::updateHealLabel(int dmg) {
    heal-=dmg;
    HEALLABEL->setText(QString::fromStdString(toString(heal)+"/"+toString(maxheal)));
    HEALLABEL->show();
    if(heal<=0){
        int W = 150;
        int H = 50;
        buttonGoBackToMenu = new QPushButton(this);
        buttonGoBackToMenu->setText("You have lost!\ngo-back-to-menu");
        buttonGoBackToMenu->setGeometry((width2 - W) / 2, 200, W, H);
        buttonGoBackToMenu->show();
        QObject::connect(buttonGoBackToMenu, &QPushButton::clicked, [&]() {
            openStartPage();
        });
        return true;
    }
    return false;
}

void startTurn() {
        for(int i = 0;i<5;i++){
            for(int j = 0;j<6;j++){
                boardFire[i][j] = false;
            }
        }
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
            int type = board[i][j];
            if(type == -1) {
                continue;
            }

            QPixmap* img = boardpic[type];
            if(boardWeather[i][j]) {
                img = boardpicWeather[type];
            }
            if(boardFire[i][j]) {
                img = boardpicFire[type];
            }
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
        damageALL[i] = 0;
    }
    //DESTROY BLOCK
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
                boardWeather[p.first][p.second] = false;
            }
            sleep(300);
        }

        while(true) {
            bool gen = false;
            for (int c = 0; c < 6; c++) {
                for (int r = 4; r > 0; r--) {
                    if (board[r][c] == -1 && board[r-1][c] != -1) {
                        swap(board[r][c],board[r-1][c]);
                        swap(boardWeather[r][c],boardWeather[r-1][c]);
                    }
                }
                if(board[0][c] == -1) {
                    board[0][c] = rand() % 6;
                    gen = true;
                }
            }
            displayBoard();
            if(!gen) {
                break;
            }
            sleep(70);
        }
        sleep(300);
    }
    for(int i = 0;i<6;i++) {
        damage[i]*=finalCombo;
        damage[i]*=dmgMultiplier;
    }
    damage[5]*=5;
    if(heal <= maxheal && damage[5]) {
        heal += damage[5];
        if(heal > maxheal) {
            heal = maxheal;
        }
        updateHealLabel(0);
    }
    sleep(300);

    vector<ENEMY*>* enemies = battles->at(level)->enemies;
    int size = enemies->size();

    //CALCULATE DAMAGE
    for(int i = 0;i<6;i++) {
        string s = selectCharacter[i]->text().toStdString();
        if(s.length() == 0){
            continue;
        }
        vector<ENEMY*>* enemies = battles->at(level)->enemies;
        int size = enemies->size();

        vector<ENEMY*>* resultEnemies = new vector<ENEMY*>();
        int finalTarget = -1;
        int type = s[0] - '0';
        if(damage[type-1] == 0) {
            continue;
        }
        if(damageALL[type-1]) {
            finalTarget = -100;
            resultEnemies = enemies;
        }

        if(finalTarget != -100) {
            bool allDead = true;
            for(int i = 0;i<size;i++) {
               allDead = enemies->at(i)->hp <= 0 && allDead;
            }

         //SELECT
         int restraintTarget = -1, normalTarget = -1, badTarget = -1;
            for(int i = 0;i<size;i++) {
                ENEMY* enemy = enemies->at(i);
                if(allDead ? enemy->dead : enemy->hp <= 0)
                    continue;
                int result = restraint(type,enemy->attr);
                if(result == 1 && restraintTarget == -1) {
                    restraintTarget = i;
                }
                if(result == 0 && normalTarget == -1) {
                    normalTarget = i;
                }
                if(result == -1 && badTarget == -1) {
                    badTarget = i;
                }
            }

            finalTarget = restraintTarget != -1 ? restraintTarget : (normalTarget != -1 ? normalTarget : badTarget);

            if(finalTarget != -1) {
                resultEnemies->push_back(enemies->at(finalTarget));
            }
        }
        qDebug() << resultEnemies->size();
        for(int i = 0;i<resultEnemies->size();i++){
            ENEMY* tg = resultEnemies->at(i);
            if(tg->dead) {
                continue;
            }
        int result = restraint(type,tg->attr);
        int finalDamage = damage[type-1] * (result == 1 ? 1.5 : (result ==0 ? 1.0 : 0.5));
        tg->hp -= finalDamage;
        QLabel* damageInt = new QLabel(this);
        damageInt->setText(QString::fromStdString(toString(finalDamage)));
        damageInt->setStyleSheet(QString::fromStdString("QLabel { color : "+color[type-1]+"; }"));
        QLabel* enemyLB = enemyLabel[tg->id];
        int rX = rand() % 21 - 10;
        int rY = rand() % 21 - 10;
        damageInt->setGeometry(tg->x-enemyLB->width()/2+rX,tg->y-enemyLB->height()/2+rY,100,10);
        damageInt->show();
        QTimer::singleShot(500, damageInt, &QWidget::hide);
        }
        sleep(150);
    }
    bool next = true;
    sleep(500);
    for(int i = 0;i<size;i++){
        ENEMY* tg = enemies->at(i);
        if(tg->hp <= 0) {
            tg->cdLabel->hide();
            enemyLabel[tg->id]->hide();
            tg->dead = 1;
        }
        next = next && tg->hp <= 0;
    }
    if(next) {
        if(level == battle-1) {
            int W = 150;
            int H = 50;
            buttonGoBackToMenu = new QPushButton(this);
            buttonGoBackToMenu->setText("Victory!\ngo-back-to-menu");
            buttonGoBackToMenu->setGeometry((width2 - W) / 2, 200, W, H);
            buttonGoBackToMenu->show();
            QObject::connect(buttonGoBackToMenu, &QPushButton::clicked, [&]() {
                openStartPage();
            });
            return;
        }
        level++;
        loadSpecialPerRound();
        displayBoard();
        showEnemy();
    }
    updateCD();
    loadSpecialPerRound();
    displayBoard();
    startTurn();
}

void MainWindow::callMouseRelease(QMouseEvent* event){
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
        destory();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    callMouseRelease(event);
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
        if(fireDamage) {
            if(boardFire[x2][y2]) {
                if(updateHealLabel(30))
                    return;
            }
            boardFire[selectX][selectY] = true;
            fireQueue->push_back({selectX,selectY});
            if(fireQueue->size() == 6) {
                bool c = false;
                pair<int,int> toRemove = fireQueue->front();
                fireQueue->pop_front();
                for(int i = 0;i<fireQueue->size();i++){
                    if(fireQueue->at(i).first == toRemove.first && fireQueue->at(i).second == toRemove.second){
                        c = true;
                        break;
                    }
                }
                if(!c) {
                    boardFire[toRemove.first][toRemove.second] = false;
                }
            }
        }
        swap(board[selectX][selectY], board[x2][y2]);
        displayBoard();
        selectX = x2;
        selectY = y2;
        lapx = x2;
        lapy = y2;
        boardLabel[x2][y2]->hide();
        if(boardWeather[selectX][selectY]) {
            if(updateHealLabel(100))
                return;
            boardWeather[selectX][selectY] = false;
            callMouseRelease(event);
        }
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
                damageALL[board[i][j]] = damageALL[board[i][j]] || v->size() >= 5;
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
        if(enemy->hp<=0)
            continue;
        enemy->cdLabel->show();
        enemy->cd--;
        enemy->cdLabel->setText(QString::fromStdString(toString(enemy->cd)));
        if(enemy->cd == 0) {
            heal-=enemy->atk;
            updateHealLabel(0);
            if(heal <= 0){
                //die
            }
            enemy->cd = enemy->defcd;
            enemy->cdLabel->setText(QString::fromStdString(toString(enemy->cd)));
            sleep(300);
        }
    }
}
void MainWindow::loadEnemy(){
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
    for(int i = 0;i<battles->size();i++){
        for(int j = 0;j<battles->at(i)->enemies->size();j++){
            battles->at(i)->enemies->at(j)->cdLabel = new QLabel(this);
            battles->at(i)->enemies->at(j)->healthLabel = new QLabel(this);
        }
    }

    for(int i = 0;i<7;i++) {
        enemyLabel[i] = new QLabel(this);
        enemyPic[i] = new QPixmap((resPath+"enemy/" + enemiesName[i] + ".png").c_str());
        *enemyPic[i] = enemyPic[i]->scaled(
            enemyPic[i]->width() / 2, enemyPic[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
        enemyLabel[i]->setPixmap(*enemyPic[i]);
        enemyLabel[i]->hide();
    }
}
void MainWindow::showEnemy() {
    vector<ENEMY*>* enemies = battles->at(level)->enemies;
    int size = enemies->size();
    for(int i = 0;i<size;i++) {
        ENEMY* enemy = enemies->at(i);
        int w = enemyPic[enemy->id]->width();
        int h = enemyPic[enemy->id]->height();
        int side = (width2 - w * size) / (size + 1);
        int finalWidth = side * (i + 1) + w * i;
        enemyLabel[enemy->id]->setGeometry(finalWidth, 80-h/2, w,h);
        enemy->x = finalWidth + w;
        enemy->y = 80-h/2 + h;
        enemyLabel[enemy->id]->show();
        //cd
        enemy->cdLabel->setText(QString::fromStdString(toString(enemy->cd)));
        enemy->cdLabel->setGeometry(finalWidth-5, 80-h/2, 100,10);
        enemy->cdLabel->setStyleSheet("QLabel { color : red; }");
        enemy->cdLabel->show();

        if(enemy->special){
            if(enemy->special == 1) {
                for(int i = 0;i<5;i++){
                    for(int j = 0;j<6;j++){
                        boardWeather[i][j] = false;
                    }
                }
            }
        }
    }
}
void MainWindow::loadSpecialPerRound() {
    fireDamage = false;
    //SPECIAL
    vector<ENEMY*>* enemies = battles->at(level)->enemies;
    int size = enemies->size();
    for(int i = 0;i<size;i++) {
        ENEMY* enemy = enemies->at(i);
        if(enemy->special == 1) {
                vector<pair<int,int>*>* avaialble = new vector<pair<int,int>*>();
                for(int r = 0;r<5;r++){
                    for(int c = 0;c<6;c++) {
                        if(!boardWeather[r][c]) {
                            avaialble->push_back(new pair<int,int>(r,c));
                        }
                    }
                }
                int r = 2;
                while(r--) {
                    if(avaialble->size() == 0) {
                        break;
                    }
                    int index = rand() % avaialble->size();
                    pair<int,int>* i = avaialble->at(index);
                    boardWeather[i->first][i->second] = true;
                    avaialble->erase(avaialble->begin()+index);
                }
        }
        if(enemy->special == 2) {
            fireDamage = true;
        }
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

    HEALLABEL->show();
    for(int i = 0;i<6;i++) {
        string s = selectCharacter[i]->text().toStdString();
        if(s.length() != 0) {
            QPixmap* img = characterPic[s[0] - '0' - 1];
            characterLabel[i]->setPixmap(*img);
            ballHeight = img->height();
            ballWidth = img->width();
            int side = (width2 - ballWidth * teamSize) / (teamSize + 1);
            int finalWidth = side * (i + 1) + ballWidth * i;
            int finalHeight = height2 - (6.5) * (side + ballHeight);
            characterLabel[i]->setGeometry(finalWidth, finalHeight, ballWidth,ballHeight);

            characterLabel[i]->show();
        }
    }
    updateHealLabel(0);
    showEnemy();
    loadSpecialPerRound();
    displayBoard();
    startTurn();

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
    if(buttonGoBackToMenu != NULL)
        buttonGoBackToMenu->hide();
    HEALLABEL->hide();
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
    loadEnemy();

    HEALLABEL = new QLabel(this);
    HEALLABEL->setGeometry(200,205,100,10);
    HEALLABEL->setStyleSheet("QLabel { color : pink; }");
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
        boardpicWeather[i] = new QPixmap((resPath+"runestone/weathered_"+stoneName[i]+".png").c_str());
        *boardpicWeather[i] = boardpicWeather[i]->scaled(
            boardpicWeather[i]->width() / 2, boardpicWeather[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
        boardpic[i] = new QPixmap((resPath+"runestone/" + stoneName[i] + ".png").c_str());
        *boardpic[i] = boardpic[i]->scaled(
            boardpic[i]->width() / 2, boardpic[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
        boardpicFire[i] = new QPixmap((resPath+"runestone/burning_" + stoneName[i] + ".png").c_str());
        *boardpicFire[i] = boardpicFire[i]->scaled(
            boardpicFire[i]->width() / 2, boardpicFire[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
    }
    //Char
    for (int j = 0; j < 6; j++) {
        characterLabel[j] = new QLabel(this);
    }
    for (int i = 0; i < 5; i++) {
        characterPic[i] = new QPixmap((resPath+"character/" + charName[i] + ".png").c_str());
        *characterPic[i] = characterPic[i]->scaled(
            characterPic[i]->width() / 2, characterPic[i]->height() / 2,
            Qt::KeepAspectRatio,      // Keep aspect ratio
            Qt::SmoothTransformation  // Smooth transformation for better quality
            );
    }

    openStartPage();
}
