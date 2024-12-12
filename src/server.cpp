#include <boost/asio.hpp>
#include <unordered_map>
#include <iostream>
#include <functional>
#include <string>
#include <chrono>
#include <thread>

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
        std::cout << "Server started on port " << port << "\n";
        start_receive();

        // ������������ ���� ���������� �����������
        cleanup_thread_ = std::thread([this]() {
            cleanup_inactive_clients();
        });
    }

    ~Server() {
        stop_cleanup_thread_ = true;
        if (cleanup_thread_.joinable()) {
            cleanup_thread_.join();
        }
    }

private:
    void start_receive() {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buffer_), remote_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::string message(recv_buffer_.data(), bytes_recvd);

                    // ���� ������� ��� � ������, ��������� ���
                    auto now = std::chrono::steady_clock::now();
                    if (connected_clients_.find(remote_endpoint_) == connected_clients_.end()) {
                        connected_clients_[remote_endpoint_] = now;
                        std::cout << "Client connected: " << remote_endpoint_ << "\n";
                    } else {
                        // �������� �����������
                        connected_clients_[remote_endpoint_] = now;
                    }

                    // ��������� ���������� ������������ �������� ���� ��������
                    send_client_count();
                }
                
                // ���������� ���� ���������� ���������
                start_receive();
            });
    }

    void send_client_count() {
        // ��������� ��������� � ����������� ��������
        std::string client_count = "Clients connected: " + std::to_string(connected_clients_.size());

        // ���������� ��������� ���� ��������
        for (const auto& [client, _] : connected_clients_) {
            socket_.async_send_to(
                boost::asio::buffer(client_count), client,
                [](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
                    // ������ �� ������ ����� ��������
                });
        }
    }

    void cleanup_inactive_clients() {
        while (!stop_cleanup_thread_) {
            auto now = std::chrono::steady_clock::now();
            for (auto it = connected_clients_.begin(); it != connected_clients_.end(); ) {
                auto last_active = it->second;
                if (std::chrono::duration_cast<std::chrono::seconds>(now - last_active).count() > 10) {
                    std::cout << "Client disconnected: " << it->first << "\n";
                    it = connected_clients_.erase(it);
                    send_client_count();
                } else {
                    ++it;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::unordered_map<udp::endpoint, std::chrono::steady_clock::time_point, EndpointHash, EndpointEqual> connected_clients_;
    std::array<char, 1024> recv_buffer_;
    std::thread cleanup_thread_;
    bool stop_cleanup_thread_ = false;
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