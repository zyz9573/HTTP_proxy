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
int UIDPLUS(){
	try {
        // using a local lock_guard to lock mtx guarantees unlocking on destruction / exception:
        std::lock_guard<std::mutex> lck (mtx);
    }
    catch (std::logic_error&) {
        std::cout << "[exception caught]\n";
    }
    std::cout<<"UID IS "<<UID<<"\r\n";
    //sleep(10);
	UID++;
	return UID;
}
void deal_request(proxy * test_server,int client_fd,std::set<std::thread::id>* threads){
		std::cout<<"------------------------------------\r\n";
		std::cout<<"client fd is "<<client_fd<<"\r\n";
		request Http_request(UIDPLUS());

		std::cout<<"here is thread "<<std::this_thread::get_id()<<" dealing with request "<<Http_request.get_uid()<<"\r\n";
		test_server->recv_request_header(&Http_request,client_fd);
		if(Http_request.get_method().compare("GET")==0){

			int socket_fd = test_server->create_socket_fd();
			int status = test_server->connect_host(Http_request.get_hostname(),Http_request.get_portnum(),socket_fd);
			std::cout<<"GET method, connect status is "<<status<<"\r\n"<<std::endl;
			if(status==-1){
				std::cout<<"connect fail"<<std::endl;
				exit(EXIT_FAILURE);
			}	
			std::cout<<"-------------GET--------------"<<"\r\n";
			test_server->send_header(Http_request.get_request(),socket_fd);
			response Http_response(Http_request.get_uid());
			test_server->recv_response_header(&Http_response,socket_fd);
			test_server->recv_message(socket_fd,Http_response.get_content(),Http_response.get_length());
			

			//doing cahce
			


			//std::cout<<Http_response.get_content()->size()<<std::endl;
			test_server->send_header(Http_response.get_response(),socket_fd);
			//std::cout<<Http_response.get_response();
			test_server->send_message(client_fd,Http_response.get_content());
			//close(socket_fd);
		}
		else if(Http_request.get_method().compare("CONNECT")==0){
			//std::cout<<"************************\r\n";
			//std::cout<<Http_request.get_request();
			//std::cout<<"************************\r\n";
			int socket_fd = test_server->create_socket_fd();
			std::cout<<Http_request.get_port()<<std::endl;
			int status = test_server->connect_host(Http_request.get_hostname(),Http_request.get_portnum(),socket_fd);
			std::cout<<"Connect method, connect status is "<<status<<"\r\n"<<std::endl;
			//std::cout<<Http_request.get_length()<<std::endl;
			if(status==-1){
				std::cout<<"connect fail"<<std::endl;
				exit(EXIT_FAILURE);
			}
			std::cout<<"-------------CONNECT--------------\r\n";

			std::string message("200 OK\r\n\r\n");
			send(client_fd,message.c_str(),message.length(),0);		


			size_t sign = 0;
			std::cout<<"client fd is "<<client_fd<<"server fd is "<<socket_fd<<"\r\n";
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
					if(test_server->transfer_TLS(client_fd,socket_fd)==-1){close(client_fd);close(socket_fd);break ; }
				}
				else if(FD_ISSET(socket_fd,&set)){
					if(test_server->transfer_TLS(socket_fd,client_fd)==-1){close(client_fd);close(socket_fd);break ; }
				}
			}
			if(sign==0){
				std::cout<<"Tunnel closed"<<std::endl;
				close(socket_fd);	
			}		
		}
		else if(Http_request.get_method().compare("POST")==0){
			int socket_fd = test_server->create_socket_fd();
			std::cout<<Http_request.get_port()<<std::endl;
			int status = test_server->connect_host(Http_request.get_hostname(),Http_request.get_portnum(),socket_fd);
			std::cout<<"Connect method, connect status is "<<status<<"\r\n"<<std::endl;
			//std::cout<<Http_request.get_length()<<std::endl;
			if(status==-1){
				std::cout<<"connect fail"<<std::endl;
				exit(EXIT_FAILURE);
			}
			std::cout<<"-------------POST--------------\r\n";

		}	
		close(client_fd);
		std::cout<<"thread "<<std::this_thread::get_id()<<" end"<<std::endl;
		std::cout<<"------------------------------------\r\n";
		std::set<std::thread::id>::iterator it = threads->find(std::this_thread::get_id());
		if(it!=threads->end()){
			threads->erase(it);
		}
		else{
			std::cout<<"thread missing in set\r\n";
		}
}
int main(int argc, char ** argv){
	//std::string hr("CONNECT www.google.com:443 HTTP/1.1\r\nHost: www.google.com\r\n\r\n");
	proxy test_server(12345);	
	test_server.bind_addr();
	std::set<std::thread::id> threads;
	while(1){
		//std::cout<<"begin"<<std::endl;
		int client_fd = test_server.accept_connection();
		if(client_fd==-1){
			std::cout<<"accept error\r\n";
			continue ;
		}
		//deal_request(&test_server,client_fd);
		

		std::thread th(deal_request,&test_server,client_fd,&threads);
		std::set<std::thread::id>::iterator it = threads.find(th.get_id()); 
		if(it!=threads.end()){
			std::cout<<"repeat thread id\r\n";
		}
		else{
			std::cout<<"insert thread "<<th.get_id()<<"\r\n";
			threads.insert(th.get_id());
		}
		th.detach();
		//std::cout<<"end"<<std::endl;
	}
	
	return EXIT_SUCCESS;
}
