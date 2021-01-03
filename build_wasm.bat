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
set RAYLIB_PATH=C:\raylib-gcc\raylib
set RAYLIB_RELEASE_PATH=C:\raylib-gcc\raylib\src
set PATH=C:\Portable\emsdk;C:\Portable\emsdk\upstream\emscripten;C:\Portable\emsdk\node\12.18.1_64bit\bin;C:\Portable\emsdk\python\3.7.4-pywin32_64bit;C:\Portable\emsdk\java\8.152_64bit\bin;%PATH%

del /Q out\*
mkdir out
emcc main.cpp -lglfw -s USE_GLFW=3 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY --shell-file %RAYLIB_PATH%\src\shell.html -I%RAYLIB_RELEASE_PATH% %RAYLIB_RELEASE_PATH%\libraylib.bc -s LLD_REPORT_UNDEFINED -Wall -Werror -O3 -g -o out\index.html
