#include <boost/asio.hpp>
#include <unordered_set>
#include <iostream>
#include <functional>
#include <string>

using boost::asio::ip::udp;

// ���������������� ������� ���� ��� udp::endpoint
struct EndpointHash {
    std::size_t operator()(const udp::endpoint& endpoint) const noexcept {
        return std::hash<std::string>()(endpoint.address().to_string()) ^ std::hash<unsigned short>()(endpoint.port());
    }
};

// �������� ��������� ��� udp::endpoint
struct EndpointEqual {
    bool operator()(const udp::endpoint& lhs, const udp::endpoint& rhs) const noexcept {
        return lhs.address() == rhs.address() && lhs.port() == rhs.port();
    }
};

class Server {
public:
    Server(boost::asio::io_context& io_context, short port)
        : socket_(io_context, udp::endpoint(udp::v4(), port)) {
        start_receive();
    }

private:
    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::string message(recv_buffer_.data(), bytes_recvd);

                    // ���� ������� ��� � ������, ��������� ���
                    if (connected_clients_.find(remote_endpoint_) == connected_clients_.end()) {
                        connected_clients_.insert(remote_endpoint_);
                        std::cout << "Client connected: " << remote_endpoint_ << "\n";
                    }

                    // ��������� ���������� ������������ �������� ���� ��������
                    send_client_count();

                    // ��������� ���� ���������� ���������
                    start_receive();
                }
                else {
                    // ���� ������, ���������� �������
                    start_receive();
                }
            });
    }

    void send_client_count() {
        // ��������� ��������� � ����������� ��������
        std::string client_count = "Clients connected: " + std::to_string(connected_clients_.size());

        // ���������� ��������� ���� ��������
        for (const auto& client : connected_clients_) {
            socket_.async_send_to(
                boost::asio::buffer(client_count), client,
                [](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
                    // ������ �� ������ ����� ��������
                });
        }
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::unordered_set<udp::endpoint, EndpointHash, EndpointEqual> connected_clients_;
    std::array<char, 1024> recv_buffer_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 12345); // ���� �������
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}
