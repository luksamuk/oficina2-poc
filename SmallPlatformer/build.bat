mkdir bin
mkdir bin\Debug

g++ --std=c++11 -g --std=c++11 -DNO_SCHEME -DLUA_MODE src/main.cpp -I"C:/Program Files (x86)/OficinaFramework2/include" -L"C:/Program Files (x86)/OficinaFramework2/lib"  -loficina2 -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lopengl32 -lglew32 -llua53 -o bin\Debug\SmallPlatformer.exe

