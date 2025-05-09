#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>

// 游戏区域大小
#define WIDTH 40
#define HEIGHT 25
#define CELL_SIZE 24
#define WINDOW_WIDTH (WIDTH * CELL_SIZE)
#define WINDOW_HEIGHT (HEIGHT * CELL_SIZE)

// 方向定义
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// 定义蛇的最大长度
#define MAX_LENGTH 100

// 全局变量区域添加
bool autoMode = true;  // 自动模式开关

// 颜色定义
SDL_Color BG_COLOR = {0, 0, 0, 255};           // 黑色背景
SDL_Color SNAKE_HEAD_COLOR = {0, 255, 0, 255}; // 绿色蛇头
SDL_Color SNAKE_BODY_COLOR = {0, 200, 0, 255}; // 浅绿色蛇身
SDL_Color FOOD_COLOR = {255, 0, 0, 255};       // 红色食物
SDL_Color BORDER_COLOR = {100, 100, 100, 255}; // 灰色边界
SDL_Color TEXT_COLOR = {255, 255, 255, 255};   // 白色文字

// 定义蛇的结构
typedef struct {
    int x[MAX_LENGTH];        // 目标位置X
    int y[MAX_LENGTH];        // 目标位置Y
    float animX[MAX_LENGTH];  // 动画插值位置X
    float animY[MAX_LENGTH];  // 动画插值位置Y
    int length;
    int direction;
    float animProgress;       // 动画进度 (0.0-1.0)
} Snake;

// 定义食物结构
typedef struct {
    int x;
    int y;
} Food;

// 全局变量
Snake snake;
Food food;
int score = 0;
bool gameOver = false;
bool gameRunning = true;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;  

// 函数声明
void initSDL();
void closeSDL();
void initGame();
void drawGame();
void handleInput();
void updateGame();
void updateAnimation();
void generateFood();
bool checkCollision();
void renderText(const char* text, int x, int y, int size);
float easeInOut(float t);
float elasticEaseOut(float t);
void drawRoundedRect(SDL_Renderer* renderer, int x, int y, int w, int h, int radius, SDL_Color color);
void autoPlay(); // 自动模式函数声明
bool isMoveSafe(int direction); // 检查移动是否安全
int countOpenSpace(int startX, int startY, int maxDepth); // 计算开放空间
bool isOccupied(int x, int y); // 检查位置是否被占用

// 自动寻路函数 贪心算法 
// https://zh.wikipedia.org/zh-cn/%E8%B4%AA%E5%BF%83%E7%AE%97%E6%B3%95#:~:text=%E8%B4%AA%E5%BF%83%E7%AE%97%E6%B3%95%EF%BC%88%E8%8B%B1%E8%AF%AD%EF%BC%9Agreedy%20algorithm,%E5%B0%B1%E6%98%AF%E4%B8%80%E7%A7%8D%E8%B4%AA%E5%BF%83%E7%AE%97%E6%B3%95%E3%80%82&text=%E8%B4%AA%E5%BF%83%E7%AE%97%E6%B3%95%E5%9C%A8%E6%9C%89%E6%9C%80,%E7%9A%84%E9%97%AE%E9%A2%98%E4%B8%AD%E5%B0%A4%E4%B8%BA%E6%9C%89%E6%95%88%E3%80%82
// void autoPlay() 
// {
//     if (!autoMode || gameOver) return;
    
//     // 获取当前蛇头位置
//     int headX = snake.x[0];
//     int headY = snake.y[0];
    
//     // 计算蛇头与食物的相对位置
//     int deltaX = food.x - headX;
//     int deltaY = food.y - headY;
    
//     // 存储可能的移动方向
//     int possibleDirections[4] = {0};
//     int dirCount = 0;
    
//     // 优先考虑X轴移动
//     if (deltaX > 0 && snake.direction != LEFT) 
//     {
//         possibleDirections[dirCount++] = RIGHT;
//     } 
//     else if (deltaX < 0 && snake.direction != RIGHT) 
//     {
//         possibleDirections[dirCount++] = LEFT;
//     }
    
//     // 其次考虑Y轴移动
//     if (deltaY > 0 && snake.direction != UP) 
//     {
//         possibleDirections[dirCount++] = DOWN;
//     } 
//     else if (deltaY < 0 && snake.direction != DOWN) 
//     {
//         possibleDirections[dirCount++] = UP;
//     }
    
//     // 如果没有可行的方向，考虑水平或垂直移动
//     if (dirCount == 0) 
//     {
//         if (snake.direction != LEFT && snake.direction != RIGHT) 
//         {
//             // 当前垂直移动，尝试水平移动
//             if (isMoveSafe(RIGHT)) 
//             {
//                 possibleDirections[dirCount++] = RIGHT;
//             } 
//             else if (isMoveSafe(LEFT)) 
//             {
//                 possibleDirections[dirCount++] = LEFT;
//             }
//         } 
//         else 
//         {
//             // 当前水平移动，尝试垂直移动
//             if (isMoveSafe(UP)) 
//             {
//                 possibleDirections[dirCount++] = UP;
//             } 
//             else if (isMoveSafe(DOWN)) 
//             {
//                 possibleDirections[dirCount++] = DOWN;
//             }
//         }
//     }
    
//     // 检查选择的方向是否安全（不会撞到蛇身）
//     for (int i = 0; i < dirCount; i++) 
//     {
//         if (isMoveSafe(possibleDirections[i])) 
//         {
//             snake.direction = possibleDirections[i];
//             return;
//         }
//     }
    
//     // 如果以上方向都不安全，尝试任意安全方向
//     if (snake.direction != UP && isMoveSafe(DOWN)) 
//     {
//         snake.direction = DOWN;
//     } 
//     else if (snake.direction != DOWN && isMoveSafe(UP)) 
//     {
//         snake.direction = UP;
//     } 
//     else if (snake.direction != LEFT && isMoveSafe(RIGHT)) 
//     {
//         snake.direction = RIGHT;
//     } 
//     else if (snake.direction != RIGHT && isMoveSafe(LEFT)) 
//     {
//         snake.direction = LEFT;
//     }
    
//     // 如果没有安全方向，保持当前方向（可能会导致游戏结束）
// }

// 改进自动寻路函数
void autoPlay() 
{
    if (!autoMode || gameOver) return;
    
    // 获取当前蛇头位置
    int headX = snake.x[0];
    int headY = snake.y[0];
    
    // 获取蛇尾位置
    int tailX = snake.x[snake.length - 1];
    int tailY = snake.y[snake.length - 1];
    
    // 计算蛇头与食物的相对位置
    int deltaX = food.x - headX;
    int deltaY = food.y - headY;
    
    // 判断是否处于紧急状态 (安全方向少于2个)
    int safeDirections = 0;
    if (isMoveSafe(UP)) safeDirections++;
    if (isMoveSafe(DOWN)) safeDirections++;
    if (isMoveSafe(LEFT)) safeDirections++;
    if (isMoveSafe(RIGHT)) safeDirections++;
    
    bool emergency = (safeDirections < 2);
    
    // 在紧急状态下，尝试跟随尾巴
    if (emergency) 
    {
        // 计算到尾巴的方向
        int tailDeltaX = tailX - headX;
        int tailDeltaY = tailY - headY;
        
        // 优先选择朝向尾巴的方向
        if (tailDeltaX > 0 && snake.direction != LEFT && isMoveSafe(RIGHT)) 
        {
            snake.direction = RIGHT;
            return;
        } 
        else if (tailDeltaX < 0 && snake.direction != RIGHT && isMoveSafe(LEFT)) {
            snake.direction = LEFT;
            return;
        }
        else if (tailDeltaY > 0 && snake.direction != UP && isMoveSafe(DOWN)) 
        {
            snake.direction = DOWN;
            return;
        }
        else if (tailDeltaY < 0 && snake.direction != DOWN && isMoveSafe(UP)) 
        {
            snake.direction = UP;
            return;
        }
    }
    
    // 存储可能的移动方向
    int possibleDirections[4] = {0};
    int dirCount = 0;
    
    // 保存每个方向的评分 (越高越好)
    int dirScores[4] = {0, 0, 0, 0}; // UP, DOWN, LEFT, RIGHT
    
    // 评估四个方向
    if (isMoveSafe(UP)) 
    {
        possibleDirections[dirCount++] = UP;
        // 如果向上移动离食物更近，加分
        if (deltaY < 0) dirScores[0] += 2;
    }
    
    if (isMoveSafe(DOWN)) 
    {
        possibleDirections[dirCount++] = DOWN;
        if (deltaY > 0) dirScores[1] += 2;
    }
    
    if (isMoveSafe(LEFT)) 
    {
        possibleDirections[dirCount++] = LEFT;
        if (deltaX < 0) dirScores[2] += 2;
    }
    
    if (isMoveSafe(RIGHT)) 
    {
        possibleDirections[dirCount++] = RIGHT;
        if (deltaX > 0) dirScores[3] += 2;
    }
    
    // 检查每个方向移动后的开放空间
    for (int i = 0; i < dirCount; i++) 
    {
        int dir = possibleDirections[i];
        int nextX = headX;
        int nextY = headY;
        
        switch (dir) 
        {
            case UP:    nextY--; break;
            case DOWN:  nextY++; break;
            case LEFT:  nextX--; break;
            case RIGHT: nextX++; break;
        }
        
        // 处理穿墙
        if (nextX < 0) nextX = WIDTH - 1;
        if (nextX >= WIDTH) nextX = 0;
        if (nextY < 0) nextY = HEIGHT - 1;
        if (nextY >= HEIGHT) nextY = 0;
        
        // 计算该位置的开放空间
        int openSpace = countOpenSpace(nextX, nextY, 5);
        
        // 根据开放空间加分 (最多加5分)
        if (dir == UP) dirScores[0] += (openSpace > 5) ? 5 : openSpace;
        else if (dir == DOWN) dirScores[1] += (openSpace > 5) ? 5 : openSpace;
        else if (dir == LEFT) dirScores[2] += (openSpace > 5) ? 5 : openSpace;
        else if (dir == RIGHT) dirScores[3] += (openSpace > 5) ? 5 : openSpace;
    }
    
    // 选择评分最高的方向
    int bestDir = -1;
    int bestScore = -1;
    
    for (int i = 0; i < dirCount; i++) 
    {
        int dir = possibleDirections[i];
        int score = 0;
        
        if (dir == UP) score = dirScores[0];
        else if (dir == DOWN) score = dirScores[1];
        else if (dir == LEFT) score = dirScores[2];
        else if (dir == RIGHT) score = dirScores[3];
        
        // 避免与当前方向相反
        if ((dir == UP && snake.direction == DOWN) ||
            (dir == DOWN && snake.direction == UP) ||
            (dir == LEFT && snake.direction == RIGHT) ||
            (dir == RIGHT && snake.direction == LEFT)) 
        {
            score -= 10; // 大幅降低掉头的评分
        }
        
        // 保持当前方向有加分
        if (dir == snake.direction) 
        {
            score += 1;
        }
        
        if (score > bestScore) 
        {
            bestScore = score;
            bestDir = dir;
        }
    }
    
    // 如果找到最佳方向，就采用
    if (bestDir != -1) 
    {
        snake.direction = bestDir;
    } 
    else 
    {
        // 如果没有找到安全方向，随机选择一个方向
        // (这种情况一般是没有任何安全方向了，游戏很快会结束)
        int randDir = rand() % 4 + 1; // 1-4
        snake.direction = randDir;
    }
}

// 计算某个位置的开放空间大小
int countOpenSpace(int startX, int startY, int maxDepth) 
{
    // 如果起始位置就不安全，返回0
    if (isOccupied(startX, startY)) return 0;
    
    int count = 0;
    int depth = 0;
    int visited[WIDTH][HEIGHT] = {0};
    
    // 创建队列进行广度优先搜索
    typedef struct {
        int x, y, depth;
    } QueueItem;
    
    QueueItem queue[WIDTH * HEIGHT];
    int front = 0, rear = 0;
    
    // 添加起始点
    queue[rear].x = startX;
    queue[rear].y = startY;
    queue[rear].depth = 0;
    rear++;
    visited[startX][startY] = 1;
    
    // 四个方向: 上、下、左、右
    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    
    while (front < rear) 
    {
        QueueItem current = queue[front++];
        count++;
        
        // 达到最大深度就停止
        if (current.depth >= maxDepth) continue;
        
        // 检查四个方向
        for (int i = 0; i < 4; i++) 
        {
            int newX = current.x + dx[i];
            int newY = current.y + dy[i];
            
            // 处理穿墙
            if (newX < 0) newX = WIDTH - 1;
            if (newX >= WIDTH) newX = 0;
            if (newY < 0) newY = HEIGHT - 1;
            if (newY >= HEIGHT) newY = 0;
            
            // 如果未访问且不是蛇身
            if (!visited[newX][newY] && !isOccupied(newX, newY)) 
            {
                visited[newX][newY] = 1;
                queue[rear].x = newX;
                queue[rear].y = newY;
                queue[rear].depth = current.depth + 1;
                rear++;
            }
        }
    }
    
    return count;
}

// 检查位置是否被占用(蛇身)
bool isOccupied(int x, int y) 
{
    for (int i = 0; i < snake.length; i++) 
    {
        if (x == snake.x[i] && y == snake.y[i]) 
        {
            return true;
        }
    }
    return false;
}

// 检查移动是否安全
bool isMoveSafe(int direction) 
{
    // 计算下一步位置
    int nextX = snake.x[0];
    int nextY = snake.y[0];
    
    switch (direction) 
    {
        case UP:    nextY--; break;
        case DOWN:  nextY++; break;
        case LEFT:  nextX--; break;
        case RIGHT: nextX++; break;
    }
    
    // 处理穿墙
    if (nextX < 0) nextX = WIDTH - 1;
    if (nextX >= WIDTH) nextX = 0;
    if (nextY < 0) nextY = HEIGHT - 1;
    if (nextY >= HEIGHT) nextY = 0;
    
    // 检查是否会撞到蛇身
    for (int i = 1; i < snake.length; i++) 
    {
        if (nextX == snake.x[i] && nextY == snake.y[i]) 
        {
            return false;
        }
    }
    
    return true;
}

int main(int argc, char* args[]) 
{
    // 初始化随机数种子
    srand(time(NULL));
    
    // 初始化SDL
    initSDL();
    
    // 初始化游戏
    initGame();
    
    // 帧率控制
    Uint32 frameStart;
    int frameTime;
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    
    // 移动计时器
    Uint32 moveTimer = 0;
    const int moveDelay = 150;   // 每150毫秒移动一次
    
    // 用于游戏结束后延迟重启的计时器
    Uint32 gameOverTimer = 0;
    const int restartDelay = 1500;  // 1.5秒后重启
    
    // 游戏主循环
    while (gameRunning) 
    {
        frameStart = SDL_GetTicks();
        
        // 处理输入
        handleInput();
        
        // 动画更新 - 每帧都更新
        updateAnimation();
        
        // 实际游戏逻辑更新 - 按照moveDelay的频率更新
        if (!gameOver && SDL_GetTicks() - moveTimer >= moveDelay) 
        {
            updateGame();
            moveTimer = SDL_GetTicks();
        }
        
        // 自动重启游戏
        if (gameOver && autoMode) {
            if (gameOverTimer == 0) {
                gameOverTimer = SDL_GetTicks();
            } else if (SDL_GetTicks() - gameOverTimer >= restartDelay) {
                initGame();
                gameOverTimer = 0;
            }
        }
        
        // 绘制游戏
        drawGame();
        
        // 帧率控制
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) 
        {
            SDL_Delay(frameDelay - frameTime);
        }
    }
    
    // 关闭SDL
    closeSDL();
    
    return 0;
}

void initSDL() 
{
    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        printf("SDL初始化失败! 错误: %s\n", SDL_GetError());
        exit(1);
    }
    
    // 初始化SDL_ttf
    if (TTF_Init() < 0) 
    {
        printf("SDL_ttf初始化失败! 错误: %s\n", TTF_GetError());
        exit(1);
    }
    
    // 加载字体
    font = TTF_OpenFont(".\\stdg.ttf", 24); 
    if (font == NULL) 
    {
        printf("字体加载失败! 错误: %s\n", TTF_GetError());
        // 尝试使用其他常见字体
        font = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", 24); // 使用Windows中的黑体字体
        if (font == NULL) 
        {
            printf("备用字体也加载失败! 错误: %s\n", TTF_GetError());
            exit(1);
        }
    }
    
    // 创建窗口
    window = SDL_CreateWindow("贪吃蛇", 
                              SDL_WINDOWPOS_UNDEFINED, 
                              SDL_WINDOWPOS_UNDEFINED, 
                              WINDOW_WIDTH, 
                              WINDOW_HEIGHT, 
                              SDL_WINDOW_SHOWN);
    if (window == NULL) 
    {
        printf("窗口创建失败! 错误: %s\n", SDL_GetError());
        exit(1);
    }
    
    // 创建渲染器
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) 
    {
        printf("渲染器创建失败! 错误: %s\n", SDL_GetError());
        exit(1);
    }
}

void closeSDL() 
{
    // 关闭字体
    TTF_CloseFont(font);
    
    // 销毁渲染器和窗口
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    // 退出SDL_ttf和SDL
    TTF_Quit();
    SDL_Quit();
}

// 初始化游戏
void initGame() 
{
    // 初始化蛇
    snake.length = 3;
    snake.direction = RIGHT;
    snake.animProgress = 0.0f;
    
    // 初始位置在游戏区域中央
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    
    // 设置蛇头位置
    snake.x[0] = centerX;
    snake.y[0] = centerY;
    snake.animX[0] = centerX;
    snake.animY[0] = centerY;
    
    // 设置蛇身位置
    for (int i = 1; i < snake.length; i++) 
    {
        snake.x[i] = centerX - i;
        snake.y[i] = centerY;
        snake.animX[i] = centerX - i;
        snake.animY[i] = centerY;
    }
    
    // 生成第一个食物
    generateFood();
    
    // 重置状态
    score = 0;
    gameOver = false;
}

// 绘制游戏界面
void drawGame() 
{
    // 清屏 (设置背景颜色)
    SDL_SetRenderDrawColor(renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
    SDL_RenderClear(renderer);
    
    // 绘制边界
    SDL_SetRenderDrawColor(renderer, BORDER_COLOR.r, BORDER_COLOR.g, BORDER_COLOR.b, BORDER_COLOR.a);
    SDL_Rect borderRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    SDL_RenderDrawRect(renderer, &borderRect);
    
    // 绘制食物
    drawRoundedRect(renderer, 
        food.x * CELL_SIZE, 
        food.y * CELL_SIZE, 
        CELL_SIZE, CELL_SIZE, 
        5, // 圆角半径
        FOOD_COLOR);

    // 绘制蛇
    for (int i = 0; i < snake.length; i++) 
    {
        int snakeX = (int)(snake.animX[i] * CELL_SIZE);
        int snakeY = (int)(snake.animY[i] * CELL_SIZE);

        if (i == 0) 
        {
            // 蛇头外轮廓
            SDL_Color outlineColor = SNAKE_HEAD_COLOR;
            outlineColor.a = 128;
            drawRoundedRect(renderer, 
                    snakeX - 2, snakeY - 2, 
                    CELL_SIZE + 4, CELL_SIZE + 4, 
                    6, outlineColor);

            // 蛇头
            drawRoundedRect(renderer, snakeX, snakeY, CELL_SIZE, CELL_SIZE, 5, SNAKE_HEAD_COLOR);
        } 
        else 
        {
            // 蛇身
            drawRoundedRect(renderer, snakeX, snakeY, CELL_SIZE, CELL_SIZE, 5, SNAKE_BODY_COLOR);
        }
        
        // 使用插值的位置
        // SDL_Rect snakeRect = {
        //     (int)(snake.animX[i] * CELL_SIZE), 
        //     (int)(snake.animY[i] * CELL_SIZE), 
        //     CELL_SIZE, 
        //     CELL_SIZE
        // };
        // SDL_RenderFillRect(renderer, &snakeRect);
    }
    
    // 如果游戏结束，显示结束信息
    if (gameOver) 
    {
        char gameOverText[50];
        sprintf(gameOverText, "游戏结束! 得分: %d", score);
        
        // 在屏幕中央绘制文本
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_Rect textBg = {WINDOW_WIDTH/4, WINDOW_HEIGHT/2 - 30, WINDOW_WIDTH/2, 60};
        SDL_RenderFillRect(renderer, &textBg);
        
        // 渲染文本
        renderText(gameOverText, WINDOW_WIDTH/2, WINDOW_HEIGHT/2, 24);
        
        char restartText[] = "按R键重新开始，按ESC键退出";
        renderText(restartText, WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 30, 18);
    } 
    else 
    {
        // 显示当前分数
        char scoreText[50];
        sprintf(scoreText, "分数: %d | 模式: %s", score, autoMode ? "自动" : "手动");
        renderText(scoreText, 120, 40, 40);
    }
    
    // 更新屏幕
    SDL_RenderPresent(renderer);
}

// 处理输入
void handleInput() {
    SDL_Event e;
    
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            gameRunning = false;
        } 
        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_SPACE:  // 空格键切换自动/手动模式
                    autoMode = !autoMode;
                    break;
                    
                case SDLK_ESCAPE:
                    gameRunning = false;
                    break;
                    
                // 手动模式下的方向控制
                case SDLK_UP:
                case SDLK_w:
                    if (!autoMode && snake.direction != DOWN)
                        snake.direction = UP;
                    break;
                    
                case SDLK_DOWN:
                case SDLK_s:
                    if (!autoMode && snake.direction != UP)
                        snake.direction = DOWN;
                    break;
                    
                case SDLK_LEFT:
                case SDLK_a:
                    if (!autoMode && snake.direction != RIGHT)
                        snake.direction = LEFT;
                    break;
                    
                case SDLK_RIGHT:
                case SDLK_d:
                    if (!autoMode && snake.direction != LEFT)
                        snake.direction = RIGHT;
                    break;
            }
        }
    }
    
    // 在自动模式下计算下一步移动
    if (autoMode) {
        autoPlay();
    }
}

// 更新游戏状态
void updateGame() 
{
    if (gameOver) 
    {
        return;
    }
    
    // 重置动画进度
    snake.animProgress = 0.0f;
    
    // 计算新的蛇头位置
    int newX = snake.x[0];
    int newY = snake.y[0];
    
    switch (snake.direction) 
    {
        case UP:
            newY--;
            break;
        case DOWN:
            newY++;
            break;
        case LEFT:
            newX--;
            break;
        case RIGHT:
            newX++;
            break;
    }
    
    // 检查是否吃到食物
    bool ate = (newX == food.x && newY == food.y);
    
    // 如果没有吃到食物，则移动蛇身（除去最后一节）
    if (!ate) 
    {
        // 移动蛇身，但保留最后一节的位置（用于增长）
        for (int i = snake.length - 1; i > 0; i--) 
        {
            snake.x[i] = snake.x[i - 1];
            snake.y[i] = snake.y[i - 1];
        }
    } 
    else 
    {
        // 如果吃到食物，移动现有蛇身，但保留蛇尾位置
        int tailX = snake.x[snake.length - 1];
        int tailY = snake.y[snake.length - 1];
        
        // 移动现有身体部分
        for (int i = snake.length - 1; i > 0; i--) 
        {
            snake.x[i] = snake.x[i - 1];
            snake.y[i] = snake.y[i - 1];
        }
        
        // 增加长度
        snake.length++;
        
        // 将新的蛇尾部分设置为之前蛇尾的位置
        snake.x[snake.length - 1] = tailX;
        snake.y[snake.length - 1] = tailY;
        
        // 同样设置动画位置，防止从(0,0)飞来
        snake.animX[snake.length - 1] = tailX;
        snake.animY[snake.length - 1] = tailY;
        
        // 增加得分
        score += 10;
        
        // 生成新的食物
        generateFood();
    }
    
    // 记录移动前的位置（用于处理穿墙动画）
    int oldHeadX = snake.x[0];
    int oldHeadY = snake.y[0];
    
    // 更新蛇头位置
    snake.x[0] = newX;
    snake.y[0] = newY;
    
    // 实现穿墙功能
    bool teleported = false;
    
    if (snake.x[0] < 0) 
    {
        snake.x[0] = WIDTH - 1;  // 从左边出去，从右边进来
        teleported = true;
    } 
    else if (snake.x[0] >= WIDTH) 
    {
        snake.x[0] = 0;          // 从右边出去，从左边进来
        teleported = true;
    }
    
    if (snake.y[0] < 0) 
    {
        snake.y[0] = HEIGHT - 1; // 从上边出去，从下边进来
        teleported = true;
    } 
    else if (snake.y[0] >= HEIGHT) 
    {
        snake.y[0] = 0;          // 从下边出去，从上边进来
        teleported = true;
    }
    
    // 如果穿墙了，直接设置动画位置，不做过渡
    if (teleported) 
    {
        snake.animX[0] = snake.x[0];
        snake.animY[0] = snake.y[0];
    }
    
    // 检查碰撞
    gameOver = checkCollision();
}

// 更新动画
void updateAnimation() 
{
    // 每帧平滑过渡的速度（可调整，值越小动画越慢）
    const float animSpeed = 0.15f;
    
    // 更新动画进度
    snake.animProgress += animSpeed;
    if (snake.animProgress > 1.0f) 
    {
        snake.animProgress = 1.0f;
    }
    
    // 使用缓动函数计算插值因子
    float factor = easeInOut(snake.animProgress);
    
    // 保存旧位置（第一次更新时）
    static bool firstUpdate = true;
    static float oldX[MAX_LENGTH];
    static float oldY[MAX_LENGTH];
    
    if (snake.animProgress <= animSpeed || firstUpdate) 
    {
        firstUpdate = false;
        for (int i = 0; i < snake.length; i++) 
        {
            oldX[i] = snake.animX[i];
            oldY[i] = snake.animY[i];
        }
    }
    
    // 更新所有蛇身节点的插值位置
    for (int i = 0; i < snake.length; i++) 
    {
        // 检测是否有大幅度位置变化（可能是穿墙）
        float diffX = fabs(snake.x[i] - oldX[i]);
        float diffY = fabs(snake.y[i] - oldY[i]);
        
        // 如果位置变化超过半个屏幕，认为是穿墙，直接设置位置不做过渡
        if (diffX > WIDTH/2 || diffY > HEIGHT/2) 
        {
            snake.animX[i] = snake.x[i];
            snake.animY[i] = snake.y[i];
        } 
        else 
        {
            // 正常情况下使用线性插值
            snake.animX[i] = oldX[i] + factor * (snake.x[i] - oldX[i]);
            snake.animY[i] = oldY[i] + factor * (snake.y[i] - oldY[i]);
        }
    }
}

// 生成食物
void generateFood() 
{
    bool valid;
    
    do 
    {
        valid = true;
        
        // 随机生成食物位置
        food.x = rand() % WIDTH;
        food.y = rand() % HEIGHT;
        
        // 确保食物不在蛇身上
        for (int i = 0; i < snake.length; i++) 
        {
            if (food.x == snake.x[i] && food.y == snake.y[i]) 
            {
                valid = false;
                break;
            }
        }
    } while (!valid);
}

// 检查碰撞
bool checkCollision() 
{
    // 不检查是否撞墙，只检查是否撞到自己
    for (int i = 1; i < snake.length; i++) 
    {
        if (snake.x[0] == snake.x[i] && snake.y[0] == snake.y[i]) 
        {
            return true;
        }
    }
    
    return false;
}

// 渲染文本
void renderText(const char* text, int x, int y, int size) 
{
    // 创建文本表面 - 使用UTF8渲染函数
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, TEXT_COLOR);
    if (surface == NULL) 
    {
        printf("无法创建文本表面! 错误: %s\n", TTF_GetError());
        return;
    }
    
    // 创建纹理
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) 
    {
        printf("无法创建纹理! 错误: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    
    // 设置渲染位置
    SDL_Rect rect;
    rect.w = surface->w;
    rect.h = surface->h;
    rect.x = x - rect.w / 2; // 居中显示
    rect.y = y - rect.h / 2;
    
    // 渲染文本
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    
    // 释放资源
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// 缓动函数 - 平滑开始和结束
float easeInOut(float t) 
{
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

// 弹性缓动函数(有反弹效果)
float elasticEaseOut(float t) 
{
    return sin(-13.0f * M_PI_2 * (t + 1.0f)) * pow(2.0f, -10.0f * t) + 1.0f;
}

// 圆角矩形绘制函数
void drawRoundedRect(SDL_Renderer* renderer, int x, int y, int w, int h, int radius, SDL_Color color) 
{
    // 限制半径大小不超过宽高的一半
    if (radius > w / 2) radius = w / 2;
    if (radius > h / 2) radius = h / 2;
    
    // 设置绘制颜色
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // 中心区域
    SDL_Rect centerRect = {x + radius, y, w - 2 * radius, h};
    SDL_RenderFillRect(renderer, &centerRect);
    
    // 左右区域
    SDL_Rect leftRect = {x, y + radius, radius, h - 2 * radius};
    SDL_RenderFillRect(renderer, &leftRect);
    
    SDL_Rect rightRect = {x + w - radius, y + radius, radius, h - 2 * radius};
    SDL_RenderFillRect(renderer, &rightRect);
    
    // 画四个角上的四分之一圆形
    // 左上角
    for (int cy = 0; cy <= radius; cy++) 
    {
        for (int cx = 0; cx <= radius; cx++) 
        {
            float distance = sqrtf(cx * cx + cy * cy);
            if (distance <= radius) 
            {
                SDL_RenderDrawPoint(renderer, x + radius - cx, y + radius - cy);
            }
        }
    }
    
    // 右上角
    for (int cy = 0; cy <= radius; cy++) 
    {
        for (int cx = 0; cx <= radius; cx++) 
        {
            float distance = sqrtf(cx * cx + cy * cy);
            if (distance <= radius) 
            {
                SDL_RenderDrawPoint(renderer, x + w - radius + cx - 1, y + radius - cy);
            }
        }
    }
    
    // 左下角
    for (int cy = 0; cy <= radius; cy++) 
    {
        for (int cx = 0; cx <= radius; cx++) 
        {
            float distance = sqrtf(cx * cx + cy * cy);
            if (distance <= radius) 
            {
                SDL_RenderDrawPoint(renderer, x + radius - cx, y + h - radius + cy - 1);
            }
        }
    }
    
    // 右下角
    for (int cy = 0; cy <= radius; cy++) 
    {
        for (int cx = 0; cx <= radius; cx++) 
        {
            float distance = sqrtf(cx * cx + cy * cy);
            if (distance <= radius) 
            {
                SDL_RenderDrawPoint(renderer, x + w - radius + cx - 1, y + h - radius + cy - 1);
            }
        }
    }
}