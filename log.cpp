#include"log.h"
#include<iostream>
#include<cstdlib>
#include<stdio.h> //stdout

int main(int argc, char* argv[]){
	std::string logpath = "/mnt/d/590hw2/zy71/erss"; //input the path here
	Log mylog(logpath);
	mylog.add("msg one");
	mylog.add("msg two");
	mylog.add("msg three");

	return 0;
}