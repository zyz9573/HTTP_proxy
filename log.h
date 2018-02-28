#include<iostream>
#include<cstdlib>
#include<stdio.h> 
#include<sys/stat.h>
#include<sys/types.h>
#include<string>
#include<fstream>
#include<unistd.h> //chdir

class Log{
private:
	std::string path;
public:
	Log(std::string logpath):path(logpath){
		const char * cpath = path.c_str();
		mkdir(cpath, ACCESSPERMS);
	}
	void add(std::string str){
		char buf[512]; //to store the cwd
		getcwd(buf, 512);
		const char * cpath = path.c_str();
		chdir(cpath);
		std::ofstream logfile("proxy.txt", std::ios_base::out | std::ios_base::app );
		logfile << str << std::endl;
		chdir(buf);
	}
	~Log(){

	}
};
