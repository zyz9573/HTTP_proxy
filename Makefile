proxy: proxy.cpp proxy.h
	g++  -o proxy -std=gnu++11 -Wall -Werror -ggdb3 proxy.cpp -lpthread
