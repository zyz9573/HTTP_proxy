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
class response{
private:
	std::string status_line;
	std::map<std::string,std::string> kv_table;
	std::vector<char> file;
	size_t file_size; 
	size_t header_size;
	int packet_num;
	void initial(char data[]){
		std::string http_response(data);
		header_size = http_response.find_first_of("\r\n\r\n")+4;
		if(packet_num==0){
			size_t flag = http_response.find_first_of("\r\n");
			status_line = http_response.substr(0,flag);
			http_response = http_response.substr(flag+2);

			while(flag!=std::string::npos){ 
				flag = http_response.find_first_of("\n");
				std::string temp(http_response.substr(0,flag));
				std::cout<<temp<<std::endl;
				http_response = http_response.substr(flag+1);
				int colon = temp.find_first_of(":");
				kv_table.insert(std::pair<std::string,std::string>(temp.substr(0,colon),temp.substr(colon+2)));
				if(http_response.substr(2).find_first_of("\r")==std::string::npos){
					break ; 
				}
			}
		}
	}
public:
	response(){
		packet_num=0;
		file_size = 0;
		//file = std::vector<char>();
		//std::string status_line();
		//std::map<std::string,std::string> kv_table();	
	}
	void print_response(){
		std::cout<<status_line<<std::endl;
		std::map<std::string,std::string>::iterator it = kv_table.begin();
		while(it != kv_table.end()){
			std::cout<<"KEY: "<<it->first<<" VALUE: "<<it->second<<std::endl;
			it++;
		}
		for(size_t i=0;i<file_size;++i){
			std::cout<<file.at(i);
		}
		std::cout<<std::endl;
	}
	std::string get_status_line(){
		return status_line;
	}
	void update_file(char data[],size_t len){
		if(file_size==0){
			initial(data);
		}
		for(size_t i=0;i<len;++i){
			file.push_back(data[i]);
		}
		file_size = file.size();
		packet_num++;
	}
	size_t get_content_length(){
		std::map<std::string,std::string>::iterator it = kv_table.find("Content-Length");
		if(it!=kv_table.end()){
			return (size_t)atoi(it->second.c_str());
		}
		else{
			return 0;
		}
	}
	std::vector<char> get_file(){
		return file;
	}
};
class request{
private:
	std::string core;//core is request_line
	std::string original_request;
	std::map<std::string,std::string> kv_table;
	std::string content;
	size_t uid;
public:
	request(std::string http_request,size_t id){
		size_t div = http_request.find_first_of("\r\n\r\n");
		if(div!=std::string::npos && (div+4)<http_request.length()){
			std::string content = http_request.substr(div+4);
			//http_request = http_request.substr(0,div+2);
		}
		uid = id;
		original_request = http_request;
		size_t flag = http_request.find_first_of("\r\n");
		core = http_request.substr(0,flag);
		http_request = http_request.substr(flag+2);
		while(flag!=std::string::npos){ 
			flag = http_request.find_first_of("\n");
			std::string temp(http_request.substr(0,flag));
			http_request = http_request.substr(flag+1);
			int colon = temp.find_first_of(":");
			kv_table.insert(std::pair<std::string,std::string>(temp.substr(0,colon),temp.substr(colon+2)));
			if(http_request.length()<4){
				break ; 
			}
		}
	}
	void print_request(){
		std::cout<<core<<std::endl;
		std::map<std::string,std::string>::iterator it = kv_table.begin();
		while(it != kv_table.end()){
			std::cout<<"KEY: "<<it->first<<" VALUE: "<<it->second<<std::endl;
			it++;
		}
	}
	void erase_header(std::string key){
		std::map<std::string,std::string>::iterator it = kv_table.find(key);
		if(it!=kv_table.end()){
			kv_table.erase(key);		
		}
		else{
			std::cout<<"NOT SUCH KEY"<<std::endl;
		}

	}
	void add_header(std::string key, std::string value){
		value +="\r";
		kv_table.insert(std::pair<std::string,std::string>(key,value));
	}
	std::string get_request_line(){
		return core;
	}
	std::string getOriginal_request(){
		return original_request;
	}
	std::string get_new_request(){
		std::string res(core);
		res +="\r\n";
		std::map<std::string,std::string>::iterator it = kv_table.begin();
		while(it != kv_table.end()){
			res+=it->first;
			res+=": ";
			res+=it->second;
			res+="\n";
			it++;
		}
		res+="\r\n";
		return res;		
	}
};

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
	size_t send_message(std::vector<char> file,int socket_fd){
		if(socket_fd==0){
			std::cout<<"socket not established"<<std::endl;
		}		
		size_t sent=0;
		std::cout<<"rl "<<file.size()<<std::endl;
		do{
			char message[1024];
			memset(message,0,sizeof(message));
			
			if(file.size()<sizeof(message)){
				memcpy(message,&file.data()[0],file.size());
				std::cout<<"message is------------------------------------------------"<<std::endl<<message<<std::endl;
				sent+=send(socket_fd,message,file.size(),0);
			}
			else{
				memcpy(message, &file.data()[sent],sizeof(message));
				sent+=send(socket_fd,message,sizeof(message),0);	
			}
		
			std::cout<<"sent "<<sent<<std::endl;
			if(file.size()<sizeof(message)){
				break;
			}
		}
		while(sent<file.size());
		return sent;
	}

	size_t send_request(std::string request,int socket_fd){//
		
		//std::cout<<"lenth is "<<request.length()<<std::endl<<"send: "<<message<<std::endl;
		if(socket_fd==0){
			std::cout<<"socket not established"<<std::endl;
		}
		size_t sent=0;
		do{
			char message[1024];
			memset(message,0,sizeof(message));
			
			if(request.length()<sizeof(message)){
				memcpy(message, request.substr(sent,1024).c_str(),request.length());
				sent+=send(socket_fd,message,request.length(),0);
			}
			else{
				memcpy(message, request.substr(sent,1024).c_str(),sizeof(message));
				sent+=send(socket_fd,message,sizeof(message),0);	
			}
			std::cout<<"rl "<<request.length()<<std::endl;		
			std::cout<<"sent "<<sent<<std::endl;
			if(request.length()<sizeof(message)){
				break;
			}
		}
		while(sent<request.length());
		return sent;
	}
	std::string recv_message(int socket_fd,response * http_response){
		
		size_t cap=0;
		std::string res;		
		do{
			char message[1024];
			memset(message,0,sizeof(message));
			size_t temp=recv(socket_fd,&message,sizeof(message),0);
			cap+=temp;
			http_response->update_file(message,temp);
			res+=std::string(message);
			if(cap<1024){return std::string(message);}
		}
		while(cap<http_response->get_content_length());	
		//
		return res;//std::string("DONE\n");
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
