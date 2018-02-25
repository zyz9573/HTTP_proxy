//created by panjoy 2/20/2018
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <map>
#include <vector>

class request_line{
private:
	std::string original_request;
	std::string method;
	std::string agreement; 
	std::string hostname;
	std::string source;
	std::string uri;
public:
	request_line(std::string http_request){
//"GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost:people.duke.edu\r\n\r\n"
		original_request = http_request;
		std::size_t filter = http_request.find_first_of(" ");
		method = std::string(http_request.substr(0,filter));
		if(method.compare("GET")==0){
			std::size_t filter3 = http_request.find_first_of(":");
			agreement = http_request.substr(filter+1,filter3-filter-1);
			std:: size_t filter2 = http_request.find_first_of("/");
			http_request = http_request.substr(filter2+2);
			filter2 = http_request.find_first_of("/");
			hostname = std::string(http_request.substr(0,filter2));
			filter = http_request.find_first_of(" ");
			uri = std::string(http_request.substr(filter2,filter-filter2));
		}
		else if(method.compare("CONNECT")==0){
			http_request = http_request.substr(filter+1);
			filter = http_request.find_first_of(":");
			hostname = std::string(http_request.substr(0,filter));
			http_request = http_request.substr(filter+1);
			filter = http_request.find_first_of(" ");
			int port = atoi(http_request.substr(0,filter).c_str());
			if(port==80){
				agreement=std::string("http");
			}
			else if(port==443){
				agreement=std::string("https");
			}
		}

	}
	void print_request_line(){
		std::cout<<"original_request_line is "<<original_request<<std::endl;
		std::cout<<"method is "<<method<<std::endl;
		std::cout<<"agreement is "<<agreement<<std::endl;
		std::cout<<"hostname is "<<hostname<<std::endl;
		std::cout<<"uri is "<<uri<<std::endl;
	}
	std::string getHostname(){
		return hostname;
	}
	std::string getAgreement(){
		return agreement;
	}
	std::string getOriginal_request(){
		return original_request;
	}
	std::string getMethod(){
		return method;
	}
	std::string getURI(){
		return uri;
	}
	std::string getSource(){
		return source;
	}
};

class request{
private: 
};