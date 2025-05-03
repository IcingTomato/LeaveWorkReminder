# Snake 贪吃蛇

## 编译

```powershell
gcc snake.c -o snake -IC:/msys64/ucrt64/x86_64-w64-mingw32/include/SDL2 -LC:/msys64/ucrt64/x86_64-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -mwindows
```

## 环境搭建

### 下载 SDL2

- [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.32.4) 2.32.4

需要下载 `SDL2-devel-2.32.4-mingw.zip`

### 解压复制

解压后进入 `\SDL2-devel-2.32.4-mingw.zip\SDL2-2.32.4` 目录下，复制 `x86_64-w64-mingw32` 这一整个目录到 `C:\msys64\ucrt64\` 目录下，最终路径为 `C:\msys64\ucrt64\x86_64-w64-mingw32`。

然后顺带把 `\SDL2-devel-2.32.4-mingw.zip\SDL2-2.32.4\x86_64-w64-mingw32\bin` 目录下的 `SDL2.dll` 复制到 `snake.exe` 目录下。
