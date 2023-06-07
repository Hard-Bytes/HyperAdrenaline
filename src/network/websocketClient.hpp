#pragma once

#include "webMessageInterface.hpp"

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {
public:
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri, WebMessageInterface* gc);
    ~connection_metadata();

    void on_open(client * c, websocketpp::connection_hdl hdl);
    void on_fail(client * c, websocketpp::connection_hdl hdl);
    void on_close(client * c, websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl, client::message_ptr msg);

    websocketpp::connection_hdl get_hdl() const;

    int get_id() const;
    std::string get_status() const;
    bool is_connected() const;

    void record_sent_message(std::string message);

    friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);
private:
    int m_id;
    websocketpp::connection_hdl m_hdl;
    bool connected{true};
    std::string m_status;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;
    WebMessageInterface* msgInterface;
};

class websocket_endpoint {
public:
    websocket_endpoint ();
    ~websocket_endpoint();

    void connect(std::string const & uri);
    void close(std::string reason, websocketpp::close::status::value code=websocketpp::close::status::normal);
    void send(std::string message);

    connection_metadata::ptr get_metadata() const;

    void setHost(bool) noexcept;
    bool isHost() const noexcept;
    bool isOnline() noexcept;

    WebMessageInterface* msgInterface;
private:
    bool host{true};

    typedef std::map<int,connection_metadata::ptr> con_list;

    client m_endpoint;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

    con_list m_connection_list;
    int m_next_id;
    int con_id{-1};
};

using NetworkClient = websocket_endpoint;
