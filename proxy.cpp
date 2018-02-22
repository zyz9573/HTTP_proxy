//created by panjoy 2/20/2018
#include "proxy.h"
int main(int argc, char ** argv){
	std::string hr("GET http://people.duke.edu/~tkb13/courses/ece650/resources/awesome.txt HTTP/1.1\r\nHost:people.duke.edu\r\n\r\n");
	request test(hr);
	test.print_request();

	proxy test_server;
	test_server.create_socket_fd();
	int status = test_server.connect_host(test.getHostname(),test.getAgreement());
	std::cout<<"connect status is "<<status<<std::endl;
	if(status==-1){
		std::cout<<"connect fail"<<std::endl;
	}
	test_server.send_message(test.getOriginal_request());
	std::string res = test_server.recv_message();
	std::cout<<"recv message is "<<res<<std::endl;
	std::cout<<res.length()<<std::endl;
	res = test_server.recv_message();
	std::cout<<"recv message is "<<res<<std::endl;
	return EXIT_SUCCESS;
}