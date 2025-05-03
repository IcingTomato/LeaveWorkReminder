#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

// 游戏区域大小
#define WIDTH 30
#define HEIGHT 20
#define CELL_SIZE 20
#define WINDOW_WIDTH (WIDTH * CELL_SIZE)
#define WINDOW_HEIGHT (HEIGHT * CELL_SIZE)

// 方向定义
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// 定义蛇的最大长度
#define MAX_LENGTH 100

// 颜色定义
SDL_Color BG_COLOR = {0, 0, 0, 255};           // 黑色背景
SDL_Color SNAKE_HEAD_COLOR = {0, 255, 0, 255}; // 绿色蛇头
SDL_Color SNAKE_BODY_COLOR = {0, 200, 0, 255}; // 浅绿色蛇身
SDL_Color FOOD_COLOR = {255, 0, 0, 255};       // 红色食物
SDL_Color BORDER_COLOR = {100, 100, 100, 255}; // 灰色边界
SDL_Color TEXT_COLOR = {255, 255, 255, 255};   // 白色文字

// 定义蛇的结构
typedef struct {
    int x[MAX_LENGTH];
    int y[MAX_LENGTH];
    int length;
    int direction;
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
void generateFood();
bool checkCollision();
void renderText(const char* text, int x, int y, int size);

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
    const int FPS = 10;
    const int frameDelay = 1000 / FPS;
    
    // 游戏主循环
    while (gameRunning) 
    {
        frameStart = SDL_GetTicks();
        
        // 处理输入
        handleInput();
        
        // 更新游戏状态
        if (!gameOver) 
        {
            updateGame();
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
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败! 错误: %s\n", SDL_GetError());
        exit(1);
    }
    
    // 初始化SDL_ttf
    if (TTF_Init() < 0) {
        printf("SDL_ttf初始化失败! 错误: %s\n", TTF_GetError());
        exit(1);
    }
    
    // 加载字体
    font = TTF_OpenFont(".\\stdg.ttf", 24); 
    if (font == NULL) {
        printf("字体加载失败! 错误: %s\n", TTF_GetError());
        // 尝试使用其他常见字体
        font = TTF_OpenFont("C:\\Windows\\Fonts\\simhei.ttf", 24); // 使用Windows中的黑体字体
        if (font == NULL) {
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
    if (renderer == NULL) {
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
    
    // 初始位置在游戏区域中央
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    
    // 设置蛇头位置
    snake.x[0] = centerX;
    snake.y[0] = centerY;
    
    // 设置蛇身位置
    for (int i = 1; i < snake.length; i++) 
    {
        snake.x[i] = centerX - i;
        snake.y[i] = centerY;
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
    SDL_SetRenderDrawColor(renderer, FOOD_COLOR.r, FOOD_COLOR.g, FOOD_COLOR.b, FOOD_COLOR.a);
    SDL_Rect foodRect = {food.x * CELL_SIZE, food.y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &foodRect);
    
    // 绘制蛇
    for (int i = 0; i < snake.length; i++) 
    {
        if (i == 0) 
        {
            // 蛇头
            SDL_SetRenderDrawColor(renderer, SNAKE_HEAD_COLOR.r, SNAKE_HEAD_COLOR.g, SNAKE_HEAD_COLOR.b, SNAKE_HEAD_COLOR.a);
        } 
        else 
        {
            // 蛇身
            SDL_SetRenderDrawColor(renderer, SNAKE_BODY_COLOR.r, SNAKE_BODY_COLOR.g, SNAKE_BODY_COLOR.b, SNAKE_BODY_COLOR.a);
        }
        
        SDL_Rect snakeRect = {snake.x[i] * CELL_SIZE, snake.y[i] * CELL_SIZE, CELL_SIZE, CELL_SIZE};
        SDL_RenderFillRect(renderer, &snakeRect);
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
        char scoreText[20];
        sprintf(scoreText, "分数: %d", score);
        renderText(scoreText, 50, 20, 16);
    }
    
    // 更新屏幕
    SDL_RenderPresent(renderer);
}

// 处理输入
void handleInput() 
{
    SDL_Event e;
    
    while (SDL_PollEvent(&e) != 0) 
    {
        if (e.type == SDL_QUIT) 
        {
            gameRunning = false;
        } 
        else if (e.type == SDL_KEYDOWN) 
        {
            switch (e.key.keysym.sym) 
            {
                case SDLK_UP:
                case SDLK_w:
                    if (snake.direction != DOWN)
                        snake.direction = UP;
                    break;
                    
                case SDLK_DOWN:
                case SDLK_s:
                    if (snake.direction != UP)
                        snake.direction = DOWN;
                    break;
                    
                case SDLK_LEFT:
                case SDLK_a:
                    if (snake.direction != RIGHT)
                        snake.direction = LEFT;
                    break;
                    
                case SDLK_RIGHT:
                case SDLK_d:
                    if (snake.direction != LEFT)
                        snake.direction = RIGHT;
                    break;
                    
                case SDLK_r:
                    if (gameOver) 
                    {
                        initGame();
                    }
                    break;
                    
                case SDLK_ESCAPE:
                    gameRunning = false;
                    break;
            }
        }
    }
}

// 更新游戏状态
void updateGame() 
{
    if (gameOver) 
    {
        return;
    }
    
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
    
    // 移动蛇身（除去最后一节）
    for (int i = snake.length - 1; i > 0; i--) 
    {
        snake.x[i] = snake.x[i - 1];
        snake.y[i] = snake.y[i - 1];
    }
    
    // 更新蛇头位置
    snake.x[0] = newX;
    snake.y[0] = newY;
    
    // 实现穿墙功能
    if (snake.x[0] < 0) 
    {
        snake.x[0] = WIDTH - 1;  // 从左边出去，从右边进来
    } 
    else if (snake.x[0] >= WIDTH) 
    {
        snake.x[0] = 0;          // 从右边出去，从左边进来
    }
    
    if (snake.y[0] < 0) 
    {
        snake.y[0] = HEIGHT - 1; // 从上边出去，从下边进来
    } 
    else if (snake.y[0] >= HEIGHT) 
    {
        snake.y[0] = 0;          // 从下边出去，从上边进来
    }
    
    // 检查是否吃到食物
    if (snake.x[0] == food.x && snake.y[0] == food.y) 
    {
        // 增加得分
        score += 10;
        
        // 增加蛇的长度
        snake.length++;
        
        // 生成新的食物
        generateFood();
    }
    
    // 检查碰撞
    gameOver = checkCollision();
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
    for (int i = 1; i < snake.length; i++) {
        if (snake.x[0] == snake.x[i] && snake.y[0] == snake.y[i]) {
            return true;
        }
    }
    
    return false;
}

// 渲染文本
void renderText(const char* text, int x, int y, int size) {
    // 创建文本表面 - 使用UTF8渲染函数
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, TEXT_COLOR);
    if (surface == NULL) {
        printf("无法创建文本表面! 错误: %s\n", TTF_GetError());
        return;
    }
    
    // 创建纹理
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
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