clang++ -Ofast -std=c++11 -o Program Program.cpp GbCPU.cpp Memory.cpp IOController.cpp GbPPU.cpp OAM.cpp frameBuffer.cpp Renderer.cpp -L/opt/homebrew/lib -lSDL2 -I/opt/homebrew/include -D_THREAD_SAFE -ggdb3 -Wall -lSDL2_image -lSDL2_mixer -lm
