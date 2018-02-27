//created by panjoy 2/20/2018
#include "proxy.h"
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
			std::cout<<"-------------CONNECT--------------"<<std::endl;;			
		}

		close(client_fd);
	}

	return EXIT_SUCCESS;
}
