proxy: proxy.cpp proxy.h
	g++ -lpthread -o proxy -std=gnu++98 -Wall -Werror -ggdb3 proxy.cpp
