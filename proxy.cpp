//created by panjoy 2/20/2018
#include "proxy.h"
int main(int argc, char ** argv){
	//std::string hr("CONNECT www.google.com:443 HTTP/1.1\r\nHost: www.google.com\r\n\r\n");

	proxy test_server(12345);
	
	test_server.bind_addr();
	while(1){
		int client_fd = test_server.accept_connection();
		std::string hr = test_server.recv_message(client_fd);
	
		request test(hr);
		test.print_request();
		int socket_fd = test_server.create_socket_fd();
		int status = test_server.connect_host(test.getHostname(),test.getAgreement(),socket_fd);
		std::cout<<"connect status is "<<status<<std::endl;
		if(status==-1){
			std::cout<<"connect fail"<<std::endl;
		}
		test_server.send_message(test.getOriginal_request(),socket_fd);
		std::string res = test_server.recv_message(socket_fd);
		std::cout<<"recv message is "<<res<<std::endl;
		test_server.send_message(res,client_fd);
	}
	return EXIT_SUCCESS;
}