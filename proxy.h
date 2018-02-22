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
class request{
private:
	std::string original_request;
	std::string method;
	std::string agreement; 
	std::string hostname;
	std::string source;
	std::string uri;
public:
	request(std::string http_request){
//"GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost:people.duke.edu\r\n\r\n"
		original_request = http_request;
		std::size_t filter = http_request.find_first_of(" ");
		method = std::string(http_request.substr(0,filter));
		std::size_t filter3 = http_request.find_first_of(":");
		agreement = http_request.substr(filter+1,filter3-filter-1);
		std:: size_t filter2 = http_request.find_first_of("/");
		http_request = http_request.substr(filter2+2);
		filter2 = http_request.find_first_of("/");
		hostname = std::string(http_request.substr(0,filter2));
		filter = http_request.find_first_of(" ");
		uri = std::string(http_request.substr(filter2,filter-filter2));
	}
	void print_request(){
		std::cout<<"original_request is "<<original_request<<std::endl;
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

class proxy{
private:
	int host_socket_fd;
	int port;
	std::string hostname;
public:
	proxy(int port_num){
		host_socket_fd = socket(AF_INET,SOCK_STREAM,0);
		port = port_num;
		char host[64];
		gethostname(host,64);
		hostname = std::string(host);
	}
	int create_socket_fd(){
		int socket_fd = socket(AF_INET,SOCK_STREAM,0);
		return socket_fd;	
	}
	void close_socket_fd(int socket_fd){
		close(socket_fd);
		socket_fd=0;
	}
	int connect_host(std::string hostname,std::string agreement,int socket_fd){
	  	struct sockaddr_in server_in;
	  	//memset(server_in,0,sizeof(server_in));
  		server_in.sin_family = AF_INET;
  		if(agreement.compare("http")==0){
  			std::cout<<agreement<<std::endl;
  			server_in.sin_port = htons(80);
  		}
  		else if(agreement.compare("https")==0){
  			server_in.sin_port = htons(443);
  		}
  		else{
  			std::cout<<"agreement invalid"<<std::endl;
  			return -1;
  		}
  		struct hostent * host_info = gethostbyname(hostname.c_str());
  		if ( host_info == NULL ) {
    		exit(EXIT_FAILURE);
  		}
		memcpy(&server_in.sin_addr, host_info->h_addr_list[0], host_info->h_length);
		int connect_status = connect(socket_fd,(struct sockaddr *)&server_in,sizeof(server_in));
		return connect_status;
	}
	void send_message(std::string request,int socket_fd){
		char * message = new char[request.length()+1];
		std::strcpy (message, request.c_str());
		std::cout<<"send: "<<message<<std::endl;
		if(socket_fd==0){
			std::cout<<"socket not established"<<std::endl;
		}
		send(socket_fd,message,request.length(),0);
	}
	std::string recv_message(int socket_fd){
		char message[4096];
		memset(message,0,sizeof(message));
		int cap=0;
		std::string res;
		do{
			cap = recv(socket_fd,&message,sizeof(message),0);
			res+=std::string(message);
			memset(message,0,sizeof(message));
		}
		while(cap>=1460);
		
		//std::cout<<message<<std::endl;
		return res;
	}
	void bind_addr(){
		struct hostent * host_info = gethostbyname(hostname.c_str());
		struct sockaddr_in server_in;
  		server_in.sin_family = AF_INET;
  		server_in.sin_port = htons(port);
		memcpy(&server_in.sin_addr, host_info->h_addr_list[0], host_info->h_length);
		int bind_status = bind(host_socket_fd,(struct sockaddr *)&server_in,sizeof(server_in));
		if(bind_status<0){
			std::cout<<"bind fail"<<std::endl;
		}
		listen(host_socket_fd,5);
	}
	int accept_connection(){
		struct sockaddr_in incoming;
		socklen_t len = sizeof(incoming);
        return accept(host_socket_fd,(struct sockaddr*)&incoming,&len);
	}
};
/*  

*/