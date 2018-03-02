//created by panjoy 2/20/2018
#include "proxy.h"
void test(std::string temp){
	std::cout<<temp<<"\r\n";
}
int maxfdp(int a, int b){
	if(a>b){return a;}
	return b;
}
long max(long a, long b){
	if(a>b){return a;}
	return b;
}
std::string get_UTC_time(long a){
	time_t now;
	struct tm * timeinfo;
	time(&now);
	std::cout<<now<<std::endl;	
	now+=a;
	timeinfo = gmtime(&now);
	std::string res(asctime(timeinfo));//with \r\n
	return res.substr(0,res.length()-1);
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
void deal_request(proxy * test_server,int client_fd,std::set<std::thread::id>* threads, Cache * cache,Log * log,struct sockaddr_in * incoming){
		std::cout<<"------------------------------------\r\n";
		std::cout<<"client fd is "<<client_fd<<"\r\n";
		request Http_request(UIDPLUS());

		std::cout<<"here is thread "<<std::this_thread::get_id()<<" dealing with request "<<Http_request.get_uid()<<"\r\n";
		try{
			test_server->recv_request_header(&Http_request,client_fd);
			std::string info1(std::to_string(Http_request.get_uid()));
			info1 = info1 +": \"";
			info1 = info1+Http_request.get_request_line()+"\" from ";
			struct in_addr ip;
			memcpy(&ip, &(incoming->sin_addr), 4);
			info1 = info1 + inet_ntoa(ip) + " @ " + get_UTC_time(0);
			log->add(info1);		
		}
		catch(std::string err){
			err= std::to_string(Http_request.get_uid()) +": NOTE " +err;
			log->add(err);
			return ;
		}
		/*

		*/
	

		if(Http_request.get_method().compare("GET")==0){

			int socket_fd = test_server->create_socket_fd();
			int status = test_server->connect_host(Http_request.get_hostname(),Http_request.get_portnum(),socket_fd);
			std::cout<<"GET method, connect status is "<<status<<"\r\n"<<std::endl;
			if(status==-1){
				std::cout<<"connect fail"<<std::endl;
				close(socket_fd);
				close(client_fd);
				return ;
			}	
			std::cout<<"-------------GET--------------"<<"\r\n";

			std::cout<<Http_request.get_request();

			cache_control req_cc;
			Http_request.set_cache_info(&req_cc);
			if(req_cc.get_nocache() && req_cc.get_nostore()){
				//nothing with cache
				test_server->send_header(Http_request.get_request(),socket_fd);

				std::string info2(std::to_string(Http_request.get_uid()));
				info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
				log->add(info2);

				response * Http_response = new response(Http_request.get_uid());
				test_server->recv_response_header(Http_response,socket_fd);
				test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

				std::string info3(std::to_string(Http_response->get_uid()));
				info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
				log->add(info3);
				if(Http_response->get_status()==200){
					std::string info4(std::to_string(Http_response->get_uid()));
					info4 = info4 + ": not cacheable because request say no cache and no store";
					log->add(info4);
				}

				test_server->send_header(Http_response->get_response(),client_fd);
				test_server->send_message(client_fd,Http_response->get_content());

				std::string info6(std::to_string(Http_response->get_uid()));
				info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
				std::cout<<info6<<std::endl;
				log->add(info6);
				

				delete Http_response;
			}
			else if(req_cc.get_nostore()){
				//can use cache but you can not store response
				response * exist_response = cache->find(Http_request.get_URI());
				if(exist_response ==NULL){
					//no useful cache
					
					std::string info5(std::to_string(Http_request.get_uid()));
					info5 = info5 +": not in cache";
					log->add(info5);

					test_server->send_header(Http_request.get_request(),socket_fd);

					std::string info2(std::to_string(Http_request.get_uid()));
					info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
					log->add(info2);

					response * Http_response = new response(Http_request.get_uid());
					test_server->recv_response_header(Http_response,socket_fd);
					test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

					std::string info3(std::to_string(Http_response->get_uid()));
					info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
					log->add(info3);
					if(Http_response->get_status()==200){
						std::string info4(std::to_string(Http_response->get_uid()));
						info4 = info4 + ": not cacheable because request say no store";
						log->add(info4);
					}
					test_server->send_header(Http_response->get_response(),client_fd);
					test_server->send_message(client_fd,Http_response->get_content());

					std::string info6(std::to_string(Http_response->get_uid()));
					info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
					log->add(info6);
				

					delete Http_response;					
				}
				else{
					//have this response, parse it to see whether it is fresh
					std::string info5(std::to_string(Http_request.get_uid()));
					info5 = info5 +": in cache";
					cache_control eres;//exist response
					exist_response->set_cache_info(&eres);
					if(req_cc.get_mustreval() || eres.get_mustreval()){
						//request say must revalidate
						
						info5+=", requires validation";
						log->add(info5);

						if(eres.get_etag().length()>0){
							//has etag
							std::string temp = "If-None-Match: "+eres.get_etag();
							Http_request.add_kv(temp);
						}
						test_server->send_header(Http_request.get_request(),socket_fd);

						std::string info2(std::to_string(Http_request.get_uid()));
						info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
						log->add(info2);

						response * Http_response = new response(Http_request.get_uid());
						test_server->recv_response_header(Http_response,socket_fd);
						test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

						std::string info3(std::to_string(Http_response->get_uid()));
						info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
						log->add(info3);
						if(Http_response->get_status()==200){
							std::string info4(std::to_string(Http_response->get_uid()));
							info4 = info4 + ": not cacheable because request say no store";
							log->add(info4);
						}
						if(Http_response->get_status()==304){
							test("304304304304304304304304304304304304304304304304304304304304304304304304304304304304\r\n");
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);
						}
						else{
							test_server->send_header(Http_response->get_response(),client_fd);
							test_server->send_message(client_fd,Http_response->get_content());

							std::string info6(std::to_string(Http_response->get_uid()));
							info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
							log->add(info6);
						}
						delete Http_response;
					}
					else{
						//can directly send back if fresh
						long now = (long)get_current_time();
						if(eres.get_maxage()>0 && eres.get_date()>0 && (eres.get_maxage()+eres.get_date())<now){	
							//fresh
							info5+=", valid";
							log->add(info5);
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);								
						}	
						else if(eres.get_maxage()==0 && eres.get_expires()>now){
							//fresh
							info5+=", valid";
							log->add(info5);
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());	
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);								
						}
						else if(eres.get_maxage()==0 && eres.get_expires()==0){
							//fresh
							info5+=", valid";
							log->add(info5);
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());	
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);
						}
						else{
							//not fresh
							test_server->send_header(Http_request.get_request(),socket_fd);
							std::string info2(std::to_string(Http_request.get_uid()));
							info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
							log->add(info2);

							response * Http_response = new response(Http_request.get_uid());
							test_server->recv_response_header(Http_response,socket_fd);
							test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

							std::string info3(std::to_string(Http_response->get_uid()));
							info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
							log->add(info3);
							if(Http_response->get_status()==200){
								std::string info4(std::to_string(Http_response->get_uid()));
								info4 = info4 + ": not cacheable because request say no store";
								log->add(info4);
							}

							test_server->send_header(Http_response->get_response(),client_fd);
							test_server->send_message(client_fd,Http_response->get_content());

							std::string info6(std::to_string(Http_response->get_uid()));
							info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
							log->add(info6);

							delete Http_response;
						}				
					}
	
				}
			}
			else if(req_cc.get_nocache()){
				//do not use cache but you can store
				test_server->send_header(Http_request.get_request(),socket_fd);

				std::string info2(std::to_string(Http_request.get_uid()));
				info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
				log->add(info2);

				response * Http_response = new response(Http_request.get_uid());
				test_server->recv_response_header(Http_response,socket_fd);
				test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

				std::string info3(std::to_string(Http_response->get_uid()));
				info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
				log->add(info3);

				test_server->send_header(Http_response->get_response(),client_fd);
				test_server->send_message(client_fd,Http_response->get_content());

				std::string info6(std::to_string(Http_response->get_uid()));
				info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
				log->add(info6);

				//analysis this response
				cache_control res_cc;
				Http_response->set_cache_info(&res_cc);

				if(Http_response->get_status()==200){
					std::string info4(std::to_string(Http_response->get_uid()));
					if(res_cc.get_nocache() || res_cc.get_nostore() || res_cc.get_private()){
						//you can not store this response
						info4 = info4 + ": not cacheable because response do not allow";
						log->add(info4);
						delete Http_response;
					}
					else{
						cache->add_to_cache(Http_request.get_URI(),Http_response);
						if(res_cc.get_mustreval()){
							info4 = info4 +": cached, but requires re-validation";
							log->add(info4);
						}
						if(res_cc.get_expires()>0){
							info4 = std::to_string(Http_response->get_uid());
							time_t now;time(&now);
							info4 = info4 + ": cached, expires at "+ get_UTC_time(res_cc.get_expires()-(long)now);
						}
						else{
							info4 = std::to_string(Http_response->get_uid());
							info4 = info4 +": cached";
							log->add(info4);						
						}
					}						
				}
				//store end	
			}
			else{
				//request don't have limit on nocache and nostore
				response * exist_response = cache->find(Http_request.get_URI());
				if(exist_response==NULL){
					//not in cache
					test_server->send_header(Http_request.get_request(),socket_fd);

					std::string info2(std::to_string(Http_request.get_uid()));
					info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
					log->add(info2);					

					response * Http_response = new response(Http_request.get_uid());
					test_server->recv_response_header(Http_response,socket_fd);
					test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

					std::string info3(std::to_string(Http_response->get_uid()));
					info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
					log->add(info3);

					test_server->send_header(Http_response->get_response(),client_fd);
					test_server->send_message(client_fd,Http_response->get_content());

					std::string info6(std::to_string(Http_response->get_uid()));
					info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
					log->add(info6);


					cache_control res_cc;
					Http_response->set_cache_info(&res_cc);

					if(Http_response->get_status()==200){
						std::string info4(std::to_string(Http_response->get_uid()));
						if(res_cc.get_nocache() || res_cc.get_nostore() || res_cc.get_private()){
							//you can not store this response
							info4 = info4 + ": not cacheable because response do not allow";
							log->add(info4);
							delete Http_response;
						}
						else{
							cache->add_to_cache(Http_request.get_URI(),Http_response);
							if(res_cc.get_mustreval()){
								info4 = info4 +": cached, but requires re-validation";
								log->add(info4);
							}
							if(res_cc.get_expires()>0){
								info4 = std::to_string(Http_response->get_uid());
								time_t now;time(&now);
								info4 = info4 + ": cached, expires at "+ get_UTC_time(res_cc.get_expires()-(long)now);
							}
							else{
								info4 = std::to_string(Http_response->get_uid());
								info4 = info4 +": cached";
								log->add(info4);						
							}
						}						
					}
				}
				else{
					//in cache
					std::string info5(std::to_string(Http_request.get_uid()));
					info5 = info5 +": in cache";
					cache_control eres;//exist response
					exist_response->set_cache_info(&eres);
					if(req_cc.get_mustreval() || eres.get_mustreval()){
						//request say must revalidate
						
						info5+=", requires validation";
						log->add(info5);

						if(eres.get_etag().length()>0){
							//has etag
							std::string temp = "If-None-Match: "+eres.get_etag();
							Http_request.add_kv(temp);
						}
						test_server->send_header(Http_request.get_request(),socket_fd);

						std::string info2(std::to_string(Http_request.get_uid()));
						info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
						log->add(info2);

						response * Http_response = new response(Http_request.get_uid());
						test_server->recv_response_header(Http_response,socket_fd);
						test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

						std::string info3(std::to_string(Http_response->get_uid()));
						info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
						log->add(info3);
						if(Http_response->get_status()==200){
							std::string info4(std::to_string(Http_response->get_uid()));
							info4 = info4 + ": not cacheable because request say no store";
							log->add(info4);
						}
						if(Http_response->get_status()==304){
							test("304304304304304304304304304304304304304304304304304304304304304304304304304304304304\r\n");
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);
						}
						else{
							test_server->send_header(Http_response->get_response(),client_fd);
							test_server->send_message(client_fd,Http_response->get_content());

							std::string info6(std::to_string(Http_response->get_uid()));
							info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
							log->add(info6);
						}
						delete Http_response;
					}
					else{
						//can directly send back if fresh
						long now = (long)get_current_time();
						if(eres.get_maxage()>0 && eres.get_date()>0 && (eres.get_maxage()+eres.get_date())<now){	
							//fresh
							info5+=", valid";
							log->add(info5);
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);								
						}	
						else if(eres.get_maxage()==0 && eres.get_expires()>now){
							//fresh
							info5+=", valid";
							log->add(info5);
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());	
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);								
						}
						else if(eres.get_maxage()==0 && eres.get_expires()==0){
							//fresh
							info5+=", valid";
							log->add(info5);
							test_server->send_header(exist_response->get_response(),client_fd);
							test_server->send_message(client_fd,exist_response->get_content());	
							std::string info6(std::to_string(Http_request.get_uid()));
							info6 = info6+": Responding \""+exist_response->get_status_line()+"\"";
							log->add(info6);
						}
						else{
							//not fresh
							test_server->send_header(Http_request.get_request(),socket_fd);
							std::string info2(std::to_string(Http_request.get_uid()));
							info2= info2+": Requesting \""+Http_request.get_request_line()+"\" from " + Http_request.get_hostname();
							log->add(info2);

							response * Http_response = new response(Http_request.get_uid());
							test_server->recv_response_header(Http_response,socket_fd);
							test_server->recv_message(socket_fd,Http_response->get_content(),Http_response->get_length());

							std::string info3(std::to_string(Http_response->get_uid()));
							info3= info3+": Received \""+Http_response->get_status_line()+"\" from " + Http_request.get_hostname();
							log->add(info3);
							if(Http_response->get_status()==200){
								std::string info4(std::to_string(Http_response->get_uid()));
								info4 = info4 + ": not cacheable because request say no store";
								log->add(info4);
							}

							test_server->send_header(Http_response->get_response(),client_fd);
							test_server->send_message(client_fd,Http_response->get_content());

							std::string info6(std::to_string(Http_response->get_uid()));
							info6 = info6+": Responding \""+Http_response->get_status_line()+"\"";
							log->add(info6);

							delete Http_response;
						}				
					}
				}				
			}
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
				std::string temp(std::to_string(Http_request.get_uid()));
				temp+="Tunnel closed";
				log->add(temp);
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

			if(test_server->recv_message(client_fd,Http_request.get_content(),Http_request.get_length())!=0){
				std::cout<<"POST RECV FROM CLIENT ERR\r\n";
			}
			
			test_server->send_header(Http_request.get_request(),socket_fd);
			test_server->send_message(socket_fd,Http_request.get_content());
			std::cout<<"************************\r\n";
			std::cout<<Http_request.get_request();
			std::cout<<Http_request.get_content()->size()<<std::endl;
			std::cout<<"************************\r\n";
			response Http_response(Http_request.get_uid());
			test_server->recv_response_header(&Http_response,socket_fd);
			std::cout<<"^^^^^^^^^^^^^^^^^^^^^^^^\r\n";
			std::cout<<Http_response.get_response();
			std::cout<<Http_response.get_content()->size()<<std::endl;
			std::cout<<"^^^^^^^^^^^^^^^^^^^^^^^^\r\n";
			if(test_server->recv_message(socket_fd,Http_response.get_content(),Http_response.get_length())!=0){
				test_server->send_header(Http_response.get_response(),client_fd);
				//std::cout<<Http_response.get_response();
				test_server->send_message(client_fd,Http_response.get_content());				
			}
			else{
				test_server->send_header(Http_response.get_response(),client_fd);
			}
		}	
		close(client_fd);
/*		
		std::cout<<"thread "<<std::this_thread::get_id()<<" end"<<std::endl;
		std::cout<<"------------------------------------\r\n";
		std::set<std::thread::id>::iterator it = threads->find(std::this_thread::get_id());
		if(it!=threads->end()){
			threads->erase(it);
		}
		else{
			std::cout<<"thread missing in set\r\n";
		}
*/
}

int main(int argc, char ** argv){
	//std::string hr("CONNECT www.google.com:443 HTTP/1.1\r\nHost: www.google.com\r\n\r\n");
	std::string path("/mnt/d/test/HTTP_proxy");
	Log log(path);
	Cache cache(1000000);//1M cache
	proxy test_server(12345);	
	test_server.bind_addr();
	std::set<std::thread::id> threads;
	while(1){
		//std::cout<<"begin"<<std::endl;
		struct sockaddr_in incoming;
		int client_fd = test_server.accept_connection(&incoming);
		if(client_fd==-1){
			std::cout<<"accept error\r\n";
			continue ;
		}
		//deal_request(&test_server,client_fd,&threads,&cache,&log);
		

		std::thread th(deal_request,&test_server,client_fd,&threads,&cache,&log,&incoming);
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

/*
int main(int argc, char ** argv){
	std::string time("Thu, 01 Mar 2018 20:49:15 GMT");
	std::cout<<time<<std::endl;
	time_t temp = parse_gmt_time(time);
	std::cout<<temp<<std::endl;
	struct tm * timeinfo = localtime(&temp);
	std::cout<<asctime(timeinfo)<<std::endl;


	temp = get_current_time();
	std::cout<<temp<<std::endl;
	std::cout<<get_UTC_time(0);
	return EXIT_SUCCESS;
}
*/
