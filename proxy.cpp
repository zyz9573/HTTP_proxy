//created by panjoy 2/20/2018
#include "proxy.h"
int main(int argc, char ** argv){
	//std::string hr("CONNECT www.google.com:443 HTTP/1.1\r\nHost: www.google.com\r\n\r\n");

	proxy test_server(12345);	
	test_server.bind_addr();
	while(1){
		int client_fd = test_server.accept_connection();
		response http_response;
		std::string hr = test_server.recv_message(client_fd,&http_response);	
		request test(hr,1);
		test.print_request();
		request_line zyz(test.get_request_line());
		zyz.print_request_line();
		int socket_fd = test_server.create_socket_fd();
		int status = test_server.connect_host(zyz.getHostname(),zyz.getAgreement(),socket_fd);
		std::cout<<"connect status is "<<status<<"\r\n"<<std::endl;
		if(status==-1){
			std::cout<<"connect fail"<<std::endl;
		}
		if(zyz.getMethod().compare("GET")==0){
			std::cout<<"-------------GET--------------"<<std::endl;
			test_server.send_request(test.getOriginal_request(),socket_fd);
			response temp;
			std::string ans = test_server.recv_message(socket_fd,&temp);
			//std::cout<<temp.get_content_length()<<std::endl;
			temp.print_response();
			size_t sent = test_server.send_message(temp.get_file(),client_fd);
			//size_t sent = test_server.send_request(ans,client_fd);
			//
			std::cout<<sent<<std::endl;
		}
		else if(zyz.getMethod().compare("CONNECT")==0){
			std::cout<<"-------------CONNECT--------------"<<std::endl;
			std::cout<<test.getOriginal_request();
			test_server.send_request(test.getOriginal_request(),socket_fd);
			response temp;
			std::string ans = test_server.recv_message(socket_fd,&temp);
			std::cout<<ans<<std::endl;
			size_t sent = test_server.send_request("200 OK\r\n",client_fd);
			std::cout<<sent<<std::endl;			
		}

		close(client_fd);
	}

	return EXIT_SUCCESS;
}
