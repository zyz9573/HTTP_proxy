//created by panjoy 2/20/2018
#include "proxy.h"
int maxfdp(int a, int b){
	if(a>b){return a;}
	return b;
}
std::string get_UTC_time(int a){
	time_t now;
	struct tm * timeinfo;
	time(&now);	
	now+=a;
	timeinfo = gmtime(&now);
	std::string res(asctime(timeinfo));
	return res;
}
void deal_request(proxy test_server){
	while(1){
		std::cout<<"begin"<<std::endl;
		int client_fd = test_server.accept_connection();
		request Http_request(1);
		std::cout<<"here"<<std::endl;
		test_server.recv_request_header(&Http_request,client_fd);
		if(Http_request.get_method().compare("GET")==0){

			int socket_fd = test_server.create_socket_fd();
			int status = test_server.connect_host(Http_request.get_hostname(),Http_request.get_agreement(),socket_fd);
			std::cout<<"GET method, connect status is "<<status<<"\r\n"<<std::endl;
			if(status==-1){
				std::cout<<"connect fail"<<std::endl;
				exit(EXIT_FAILURE);
			}	
			std::cout<<"-------------GET--------------"<<std::endl;
			test_server.send_header(Http_request.get_request(),socket_fd);
			response Http_response(1);
			test_server.recv_response_header(&Http_response,socket_fd);
			test_server.recv_message(socket_fd,Http_response.get_content(),Http_response.get_length());
			//std::cout<<Http_response.get_content()->size()<<std::endl;
			test_server.send_header(Http_response.get_response(),socket_fd);
			std::cout<<Http_response.get_response();
			test_server.send_message(client_fd,Http_response.get_content());
		}
		else if(Http_request.get_method().compare("CONNECT")==0){
			//std::cout<<"************************\r\n";
			//std::cout<<Http_request.get_request();
			//std::cout<<"************************\r\n";
			int socket_fd = test_server.create_socket_fd();
			std::cout<<Http_request.get_port()<<std::endl;
			int status = test_server.connect_host(Http_request.get_hostname(),Http_request.get_port(),socket_fd);
			std::cout<<"Connect method, connect status is "<<status<<"\r\n"<<std::endl;
			std::cout<<Http_request.get_length()<<std::endl;
			if(status==-1){
				std::cout<<"connect fail"<<std::endl;
				exit(EXIT_FAILURE);
			}
			std::cout<<"-------------CONNECT--------------"<<std::endl;

			std::string message("200 OK\r\n\r\n");
			send(client_fd,message.c_str(),message.length(),0);		


			size_t sign = 0;
			std::cout<<"client fd is "<<client_fd<<"server fd is "<<socket_fd<<std::endl;
			while(1){
				fd_set set;
				FD_ZERO(&set);
				FD_SET(client_fd,&set);
				FD_SET(socket_fd,&set);

				struct timeval timeout;
				timeout.tv_sec = 1;
				timeout.tv_usec = 500000;//500ms
				sign = select(maxfdp(client_fd,socket_fd)+1,&set,NULL,NULL,&timeout);
				if(sign==0){break;}		
				if(FD_ISSET(client_fd,&set)){
					if(test_server.transfer_TLS(client_fd,socket_fd)==-1){close(client_fd);close(socket_fd);break ; }
				}
				else if(FD_ISSET(socket_fd,&set)){
					if(test_server.transfer_TLS(socket_fd,client_fd)==-1){close(client_fd);close(socket_fd);break ; }
				}
			}
			if(sign==0){
				std::cout<<"Tunnel closed"<<std::endl;
				close(client_fd);
				close(socket_fd);
				continue ;		
			}		
		}
		close(client_fd);	
		std::cout<<"end"<<std::endl;
	}
}
int main(int argc, char ** argv){
	//std::string hr("CONNECT www.google.com:443 HTTP/1.1\r\nHost: www.google.com\r\n\r\n");
	proxy test_server(12345);	
	test_server.bind_addr();
	deal_request(test_server);
	
	return EXIT_SUCCESS;
}
