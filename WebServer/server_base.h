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
		* resources: 对应资源的处理方式
		* default_resources: 默认的资源处理方式
		*/
		resource_type resources;
		resource_type default_resources;

		void start();
	protected:
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::acceptor acceptor;
		boost::asio::ip::tcp::endpoint endpoint;
		size_t num_threads;
		std::vector<std::thread> threads();
		virtual void accept();
		void processRequestAndResponse(std::shared_ptr<socket_type> socket);
	};

	template<typename socket_type>
	class Server : public ServerBase<socket_type> {};

	struct Request {
		// 请求行：请求方式，请求路径，http版本号
		std::string method, path, version;
		// 请求头
		std::unordered_map<std::string, std::string> headers;
		// 请求内容
		std::shared_ptr<std::iostream> content;
	};
}

#endif