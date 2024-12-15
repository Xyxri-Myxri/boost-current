#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using boost::asio::ip::udp;

/**
 * @class Client
 * @brief Класс UDP клиента для отправки сообщений серверу и получения ответов.
 */
class Client {
public:
    /**
     * @brief Конструктор клиента.
     * @param io_context Контекст ввода/вывода Boost.Asio.
     * @param host Адрес сервера.
     * @param port Порт сервера.
     */
    Client(boost::asio::io_context& io_context, const std::string& host, short port)
        : socket_(io_context, udp::endpoint(udp::v4(), 0)) {
        // Инициализация конечной точки сервера
        server_endpoint_ = *udp::resolver(io_context).resolve(udp::v4(), host, std::to_string(port)).begin();

        // Отправка первого сообщения серверу
        send_message("Hello, server!");

        // Начало асинхронного приема сообщений от сервера
        start_receive();
    }

private:
    /**
     * @brief Отправка сообщения серверу.
     * @param message Сообщение, которое необходимо отправить.
     */
    void send_message(const std::string& message) {
        socket_.async_send_to(
            boost::asio::buffer(message), server_endpoint_,
            [](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
                // Обработка завершения отправки, если необходимо
            });
    }

    /**
     * @brief Начало асинхронного получения сообщений от сервера.
     */
    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), server_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    // Обработка полученного сообщения
                    std::string message(recv_buffer_.data(), bytes_recvd);
                    update_console(message);
                }

                // Повторное ожидание следующего сообщения
                start_receive();
            });
    }

    /**
     * @brief Обновление консоли с отображением сообщения.
     * @param message Сообщение, полученное от сервера.
     */
    void update_console(const std::string& message) {
        std::cout << "\r" << message << std::flush; // Перезаписывает строку в консоли
    }

    udp::socket socket_; ///< Сокет для отправки и получения данных.
    udp::endpoint server_endpoint_; ///< Конечная точка сервера.
    std::array<char, 1024> recv_buffer_; ///< Буфер для хранения входящих данных.
};

int main() {
    try {
        boost::asio::io_context io_context;

        // Создание клиента, подключающегося к серверу по указанному адресу и порту
        Client client(io_context, "127.0.0.1", 12345);

        // Запуск обработки операций ввода/вывода
        io_context.run();
    }
    catch (std::exception& e) {
        // Обработка ошибок
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
