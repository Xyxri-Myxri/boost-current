#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using boost::asio::ip::udp;

class Client {
public:
    Client(boost::asio::io_context& io_context, const std::string& host, short port)
        : socket_(io_context, udp::endpoint(udp::v4(), 0)) {
        server_endpoint_ = *udp::resolver(io_context).resolve(udp::v4(), host, std::to_string(port)).begin();

        // Отправляем сообщение серверу для регистрации
        send_message("Hello, server!");

        // Начинаем приём сообщений от сервера
        start_receive();
    }

private:
    void send_message(const std::string& message) {
        socket_.async_send_to(
            boost::asio::buffer(message), server_endpoint_,
            [](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
                // Ничего не делаем после отправки
            });
    }

    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), server_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::string message(recv_buffer_.data(), bytes_recvd);
                    update_console(message);
                }

                // Продолжаем слушать
                start_receive();
            });
    }

    void update_console(const std::string& message) {
        // Перемещаем курсор в начало строки и выводим новое сообщение
        std::cout << "\r" << message << std::flush;
    }

    udp::socket socket_;
    udp::endpoint server_endpoint_;
    std::array<char, 1024> recv_buffer_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        Client client(io_context, "127.0.0.1", 12345); // Хост и порт сервера

        // Запускаем обработку сообщений
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
