#pragma once
#ifndef SERVER_BASE_H
#define SERVER_BASE_H

#include<string>
#include<memory>
#include<unordered_map>
#include<map>
#include<functional>
#include<boost/asio.hpp>
#include<thread>
#include<regex>


namespace WebServer{
	typedef std::map<std::string, std::unordered_map<std::string, std::function<void(std::ostream &response, Request request)>>> resource_type;
	template <typename socket_type>
	class ServerBase {
	public:
		ServerBase(unsigned short port, size_t num_threads = 1):
			endpoint(boost::asio::ip::tcp::v4(), port),
			acceptor(m_io_service, endpoint),
			num_threads(num_threads){}
		/* usage: server.resources[path][method] = function
		* resources: ��Ӧ��Դ�Ĵ���ʽ
		* default_resources: Ĭ�ϵ���Դ����ʽ
		*/
		resource_type resources;
		resource_type default_resources;

		void start() {
			for (auto it = resources.begin(); it != resources.end();it++) {
				all_resources.push_back(it);
			}
			for (auto it = default_resources.begin(); it != default_resources.end(); it++) {
				all_resources.push_back(it);
			}
			accept();
			for (size_t i = 1; i < nums_threads;i++) {
				threads.emplace_back([this]() {
					m_io_service.run();
				});
			}
			// ���߳�
			m_io_service.run();
			for (auto& t: threads) {
				t.join();
			}
		};
	protected:
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::acceptor acceptor;
		boost::asio::ip::tcp::endpoint endpoint;
		size_t num_threads;
		std::vector<std::thread> threads;
		std::vector<resource_type::iterator> all_resources;
		virtual void accept();
		void processRequestAndResponse(std::shared_ptr<socket_type> socket) const {
			auto read_buffer = std::make_shared<boost::asio::streambuf>();
			boost::asio::async_read_until(
				*socket,
				*read_buffer, 
				'\r\n\r\n',
				[this, socket, read_buffer](const boost::system::error_code& ec, size_t bytes_transferred){
				if (!ec){
					size_t total = read_buffer->size();
					std::istream stream(read_buffer.get());
					auto request = std::make_shared<Request>();
					// �������ƿ��ܲ����ã�Ӧ��Ҫ�󷵻�һ������ָ�룬�Ա������Ա���ܵĴ���ʹ��
					reqeust = parse_request()
				}
			});
		};
		Request parse_request(std::istream& istream) const {
			Request request;
			std::regex re("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
			std::smatch sub_match;
			std::string line;
			getline(istream, line);
			line.pop_back();
			if (std::regex_match(line, sub_match, re)) {
				request.method = sub_match[1];
				request.path = sub_match[2];
				request.http_version = sub_match[3];
				bool matched;
				re = "^([^:]*): ?(.*)$";
				do {
					getline(istream, line);
					line.pop_back();
					matched = std::regex_match(line, sub_match, re);
					if (matched) {
						request.headers[sub_match[1]] = sub_match[2];
					}
				} while(matched==true)
			}
			return request;
		}
	};

	template<typename socket_type>
	class Server : public ServerBase<socket_type> {};

	struct Request {
		// �����У�����ʽ������·����http�汾��
		std::string method, path, version;
		// ����ͷ
		std::unordered_map<std::string, std::string> headers;
		// ��������
		std::shared_ptr<std::iostream> content;
	};
}

#endif