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
class request{
private:
	std::string request_line;
	std::string method;
	std::string agreement; 
	std::string hostname;
	std::string uri;
	std::map<std::string,std::string> kv_table;
	std::vector<char> * content;
public:
	request(){
		content = new std::vector<char>();
	}
	void parse_request_line(std::string http_request){
		request_line = http_request;
		std::size_t filter = http_request.find_first_of(" ");
		method = std::string(http_request.substr(0,filter));
		if(method.compare("GET")==0 || method.compare("POST")==0){
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
	void add_kv(std::string temp){
		if(temp.length()==0){return ;}
		size_t colon = temp.find_first_of(":");
		if(colon == std::string::npos){
			std::cout<<"invalid KV"<<std::endl;
			return ;
		}
		kv_table.insert(std::pair<std::string,std::string>(temp.substr(0,colon),temp.substr(colon+2)));
	}
	void update_content(char data[], size_t size){
		for(size_t i=0;i<size;++i){
			content->push_back(data[i]);
		}
	}
	std::string get_request(){
		std::string res = request_line;
		std::map<std::string,std::string>::iterator it = kv_table.begin();
		while(it!=kv_table.end()){
			res+="\r\n";
			res+=it->first;
			res+=": ";
			res+=it->second;
			++it;
		}
		res+="\r\n\r\n";
		return res;
	}
	std::string get_hostname(){
		return hostname;
	}
	std::string get_agreement(){
		return agreement;
	}
	std::string get_request_line(){
		return request_line;
	}
	std::string get_method(){
		return method;
	}
	std::string get_URI(){
		return uri;
	}
	std::vector<char> * get_content(){
		return content;
	}
};
class response{
private:
	std::string status_line;
	int status;
	std::string agreement;
	std::string detail;
	std::map<std::string,std::string> kv_table;
	std::vector<char> * content;
public:
	response(){
		content = new std::vector<char>();
	}
	void parse_status_line(std::string http_response){
		status_line = http_response;
		size_t filter = http_response.find_first_of(" ");
		agreement = http_response.substr(0,filter);
		http_response = http_response.substr(filter+1);
		filter = http_response.find_first_of(" ");
		std::string temp = http_response.substr(0,filter);
		status = (size_t)atoi(temp.c_str());
		http_response = http_response.substr(filter+1);
		filter = http_response.find_first_of(" ");
		detail = http_response.substr(0,filter);
	}
	void add_kv(std::string temp){
		if(temp.length()==0){return ;}
		size_t colon = temp.find_first_of(":");
		if(colon == std::string::npos){
			std::cout<<"invalid KV"<<std::endl;
			return ;
		}
		kv_table.insert(std::pair<std::string,std::string>(temp.substr(0,colon),temp.substr(colon+2)));
	}
	void update_content(char data[], size_t size){
		for(size_t i=0;i<size;++i){
			content->push_back(data[i]);
		}
	}
	std::string get_response(){
		std::string res = status_line;
		std::map<std::string,std::string>::iterator it = kv_table.begin();
		while(it!=kv_table.end()){
			res+="\r\n";
			res+=it->first;
			res+=": ";
			res+=it->second;
			++it;
		}
		res+="\r\n\r\n";
		return res;
	}
	int get_status(){
		return status;
	}
	std::string get_status_line(){
		return status_line;
	}
	std::string get_agreement(){
		return agreement;
	}
	std::string get_detail(){
		return detail;
	}
	std::vector<char> * get_content(){
		return content;
	}
};
class cache{
private:
	std::map<std::string , response > database;
public:
	void add_to_cache(std::string request_line,response http_response){
		database.insert(std::pair<std::string,response>(request_line,http_response));
	}
	void delete_from_cache(std::string request_line){
		if(database.empty()){
			std::cout<<"cache is empty, can not delete"<<std::endl;
			return ;
		}
		std::map<std::string , response>::iterator it = database.find(request_line);
		if(it != database.end()){
			database.erase(request_line);
		}
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
	~proxy(){
	  close(host_socket_fd);
	}
	int create_socket_fd(){
		int socket_fd = socket(AF_INET,SOCK_STREAM,0);
		return socket_fd;	
	}
	void close_socket_fd(int socket_fd){
		close(socket_fd);
	}
	int connect_host(std::string hostname,std::string agreement,int socket_fd){
	  	struct sockaddr_in server_in;
	  	//memset(server_in,0,sizeof(server_in));
  		server_in.sin_family = AF_INET;
  		if(agreement.compare("http")==0){
		  //std::cout<<agreement<<std::endl;
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
	size_t send_message(int socket_fd, std::vector<char> * content){
		if(socket_fd==0){
			std::cout<<"socket not established"<<std::endl;
		}
		size_t sent=0;
		std::cout<<"size is "<<content->size()<<std::endl;
		do{
			char message[1024];
			memset(message,0,sizeof(message));
			if((content->size()-sent)<sizeof(message)){
				memcpy(message,&(content->data()[sent]),(content->size()-sent));
				sent+=send(socket_fd,message,(content->size()-sent),0);
			}
			else{
				memcpy(message,&(content->data()[sent]),sizeof(message));
				sent+=send(socket_fd,message,sizeof(message),0);	
			}
			if(content->size()<sizeof(message)){
				break;
			}
			std::cout<<sent<<std::endl;
		}
		while(sent<content->size());
		return sent;
	}
	void send_header(std::string header,int socket_fd){
		if(socket_fd==0){
			std::cout<<"socket not established"<<std::endl;
		}
		char message[1024];
		memset(message,0,sizeof(message));
		memcpy(message,header.c_str(),header.length());
		send(socket_fd,message,header.length(),0);
	}
	void recv_request_header(request * HTTP, int socket_fd){
		char message[1024];
		memset(message,0,sizeof(message));
		size_t cap=recv(socket_fd,&message,sizeof(message),0);
		std::cout<<cap<<std::endl;
		std::string temp(message);
		size_t filter=0;
		while((filter = temp.find_first_of("\r\n"))!= 0 ){
			if(HTTP->get_request_line().length()==0){
				HTTP->parse_request_line(temp.substr(0,filter));
			}
			else{
				HTTP->add_kv(temp.substr(0,filter));
			}
			temp = temp.substr(filter+2);
		}
	}
	void recv_response_header(response * HTTP, int socket_fd){
		char message[1024];
		memset(message,0,sizeof(message));
		recv(socket_fd,&message,sizeof(message),0);
		std::string temp(message);
		size_t sign=0;
		size_t filter=0;
		while((filter = temp.find_first_of("\r\n"))!= 0 ){
			if(HTTP->get_status_line().length()==0){
				HTTP->parse_status_line(temp.substr(0,filter));
			}
			else{
				HTTP->add_kv(temp.substr(0,filter));
			}
			temp = temp.substr(filter+2);
			sign+=filter;
			sign+=2;
		}
		sign+=2;
		if(sign<1024){
			for(size_t i =sign;i<1024;++i){
				HTTP->get_content()->push_back(message[i]);
			}
		}
	}
	void recv_message(int socket_fd, std::vector<char> * v){		
		size_t cap=0;	
		do{
			char message[1];
			memset(message,0,sizeof(message));
			cap=recv(socket_fd,&message,sizeof(message),0);
			v->push_back(message[0]);
		}
		while(cap!=0);
		std::cout<<"content size is "<<v->size()<<std::endl;
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
