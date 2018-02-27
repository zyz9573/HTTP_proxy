//created by panjoy 2/20/2018
#include "proxy.h"
int maxfdp(int a, int b){
	if(a>b){return a;}
	return b;
}
int main(int argc, char ** argv){
	//std::string hr("CONNECT www.google.com:443 HTTP/1.1\r\nHost: www.google.com\r\n\r\n");

	proxy test_server(12345);	
	test_server.bind_addr();
	while(1){
		int client_fd = test_server.accept_connection();
		request Http_request;
		test_server.recv_request_header(&Http_request,client_fd);
		int socket_fd = test_server.create_socket_fd();
		int status = test_server.connect_host(Http_request.get_hostname(),Http_request.get_agreement(),socket_fd);
		std::cout<<"connect status is "<<status<<"\r\n"<<std::endl;
		if(status==-1){
			std::cout<<"connect fail"<<std::endl;
			exit(EXIT_FAILURE);
		}
		if(Http_request.get_method().compare("GET")==0){
			std::cout<<"-------------GET--------------"<<std::endl;
			test_server.send_header(Http_request.get_request(),socket_fd);
			response Http_response;
			test_server.recv_response_header(&Http_response,socket_fd);
			test_server.recv_message(socket_fd,Http_response.get_content());
			std::cout<<Http_response.get_content()->size()<<std::endl;
			test_server.send_header(Http_response.get_response(),socket_fd);
			std::cout<<Http_response.get_response();
			test_server.send_message(client_fd,Http_response.get_content());
		}
		else if(Http_request.get_method().compare("CONNECT")==0){

			std::cout<<"-------------CONNECT--------------"<<std::endl;
			char message[16]="200 OK\r\n\r\n";
			send(client_fd,message,sizeof(message),0);
			std::vector<char> * content = new std::vector<char>();
			/*
			
			 for(int i=0;i<5;i++){

				test_server.recv_message(client_fd,content);
				std::cout<<"c"<<content->size()<<std::endl;
				test_server.send_message(socket_fd,content);

				content->clear();
				test_server.recv_message(socket_fd,content);
				std::cout<<"s"<<content->size()<<std::endl;
				test_server.send_message(client_fd,content);//this

				content->clear();

			}
			*/
			fd_set set;
			FD_ZERO(&set);
			FD_SET(client_fd,&set);
			FD_SET(socket_fd,&set);
			struct timeval timeout;
			timeout.tv_sec = 5;
			timeout.tv_usec = 5000;
			size_t sign = 0;

			while((sign = select(maxfdp(client_fd,socket_fd)+1,&set,NULL,NULL,&timeout))>0){
				if(FD_ISSET(client_fd,&set)){
					test_server.test_recv_message(client_fd,content);
					
					FD_CLR(client_fd,&set);
					FD_SET(client_fd,&set);
					if(content->size()==1){content->clear();continue;}
					test_server.send_message(socket_fd,content);
				}
				else if(FD_ISSET(socket_fd,&set)){
					test_server.recv_message(socket_fd,content);
					;
					FD_CLR(socket_fd,&set);
					FD_SET(socket_fd,&set);
					if(content->size()==1){content->clear();continue;}
					test_server.send_message(client_fd,content);
				}
				content->clear();
			}

			delete content;	
			
		}

		close(client_fd);	
	}
	
	return EXIT_SUCCESS;
}
