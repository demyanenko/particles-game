set EMSDK_PY=C:\Portable\emsdk\python\3.7.4-pywin32_64bit\python.exe
set PYTHONHOME=
set PYTHONPATH=
set EMSDK=C:/Portable/emsdk
set EMSDK_NODE=C:\Portable\emsdk\node\12.18.1_64bit\bin\node.exe
set EMSDK_PYTHON=C:\Portable\emsdk\python\3.7.4-pywin32_64bit\python.exe
set EM_CACHE=C:/Portable/emsdk/upstream/emscripten\cache
set EM_CONFIG=C:\Portable\emsdk\.emscripten
set EM_PY=C:\Portable\emsdk\python\3.7.4-pywin32_64bit\python.exe
set JAVA_HOME=C:\Portable\emsdk\java\8.152_64bit
set RAYLIB_RELEASE_PATH=C:\raylib-gcc\raylib\src
set PATH=C:\Portable\emsdk;C:\Portable\emsdk\upstream\emscripten;C:\Portable\emsdk\node\12.18.1_64bit\bin;C:\Portable\emsdk\python\3.7.4-pywin32_64bit;C:\Portable\emsdk\java\8.152_64bit\bin;%PATH%
set WASM_ARGS=main.cpp -lglfw -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY --shell-file shell.html -I%RAYLIB_RELEASE_PATH% %RAYLIB_RELEASE_PATH%\libraylib.bc -DWASM -Wall -O3 -g

rmdir /s /q out
mkdir out

call mkdir out\just-me
call emcc %WASM_ARGS% -DDEMO_WASM_JUSTME -o out\just-me\index.html

call mkdir out\1-bot
call emcc %WASM_ARGS% -DDEMO_WASM_1BOT -o out\1-bot\index.html

call mkdir out\2-bots
call emcc %WASM_ARGS% -DDEMO_WASM_2BOTS -o out\2-bots\index.html
