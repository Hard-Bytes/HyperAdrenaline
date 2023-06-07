#include "websocketClient.hpp"
#include "../util/macros.hpp"

/*Connection Metadata class*/
connection_metadata::connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri, WebMessageInterface* gc)
    : m_id(id)
    , m_hdl(hdl)
    , m_status("Connecting")
    , m_uri(uri)
    , m_server("N/A")
    , msgInterface(gc)
{}

connection_metadata::~connection_metadata()
{
    LOG_ONLINETMP("Empieza destructor de connection_metadata");
    LOG_ONLINETMP("Termina destructor de connection_metadata");
}

void connection_metadata::on_open(client * c, websocketpp::connection_hdl hdl) {
    LOG_ONLINETMP("Empieza on_open");
    m_status = "Open";
    connected = true;

    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    LOG_ONLINETMP("Termina on_open");
}

void connection_metadata::on_fail(client * c, websocketpp::connection_hdl hdl) {
    LOG_ONLINETMP("Empieza on_fail");
    m_status = "Failed";
    connected = false;

    client::connection_ptr con = c->get_con_from_hdl(hdl);
    m_server = con->get_response_header("Server");
    m_error_reason = con->get_ec().message();
    LOG_ONLINETMP("Termina on_fail");
}

void connection_metadata::on_close(client * c, websocketpp::connection_hdl hdl) {
    LOG_ONLINETMP("Empieza on_close");
    m_status = "Closed";
    connected = false;

    client::connection_ptr con = c->get_con_from_hdl(hdl);
    std::stringstream s;
    s << "close code: " << con->get_remote_close_code() << " (" 
        << websocketpp::close::status::get_string(con->get_remote_close_code()) 
        << "), close reason: " << con->get_remote_close_reason();
    m_error_reason = s.str();
    LOG_ONLINETMP("Termina on_close");
}

void connection_metadata::on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    LOG_ONLINETMP("Empieza on_message");
    // Start message handling
    LOG_ONLINETMP(" > Encolando mensaje");
    msgInterface->queueMessage(msg->get_payload());

    LOG_ONLINETMP(" > Hecho");
    LOG_ONLINETMP("Termina on_message");
}

websocketpp::connection_hdl connection_metadata::get_hdl() const {
    return m_hdl;
}

int connection_metadata::get_id() const {
    return m_id;
}

std::string connection_metadata::get_status() const {
    return m_status;
}

bool connection_metadata::is_connected() const {
    return connected;
}

void connection_metadata::record_sent_message(std::string message) {
    //m_messages.push_back(">> " + message);
}

std::ostream & operator<< (std::ostream & out, connection_metadata const & data) {
    out << "> URI: " << data.m_uri << "\n"
        << "> Status: " << data.m_status << "\n"
        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason) << "\n";

    return out;
}





/*Websocket Endpoint class*/
websocket_endpoint::websocket_endpoint () : m_next_id(0) {
    LOG_ONLINETMP("Empieza constructor del cliente");
    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);

    m_endpoint.init_asio();
    m_endpoint.start_perpetual();

    m_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &m_endpoint);
    LOG_ONLINETMP("Termina constructor del cliente");
}

websocket_endpoint::~websocket_endpoint() {
    LOG_ONLINETMP("Empieza destructor del cliente");
    m_endpoint.stop_perpetual();
    
    for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
        if (it->second->get_status() != "Open") {
            // Only close open connections
            continue;
        }
        
        std::cout << "> Closing connection " << it->second->get_id() << std::endl;
        
        websocketpp::lib::error_code ec;
        m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
        if (ec) {
            std::cout << "> Error closing connection " << it->second->get_id() << ": "  
                        << ec.message() << std::endl;
        }
    }
    
    m_thread->join();
    LOG_ONLINETMP("Termina destructor del cliente");
}

void websocket_endpoint::connect(std::string const & uri) {
    LOG_ONLINETMP("Empieza connect");
    websocketpp::lib::error_code ec;

    client::connection_ptr con = m_endpoint.get_connection(uri, ec);

    if (ec) {
        std::cout << "> Connect initialization error: " << ec.message() << std::endl;
        return;
    }

    int new_id = m_next_id++;
    connection_metadata::ptr metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(new_id, con->get_handle(), uri, msgInterface);
    m_connection_list[new_id] = metadata_ptr;

    con->set_open_handler(websocketpp::lib::bind(
        &connection_metadata::on_open,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));
    con->set_fail_handler(websocketpp::lib::bind(
        &connection_metadata::on_fail,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));
    con->set_close_handler(websocketpp::lib::bind(
        &connection_metadata::on_close,
        metadata_ptr,
        &m_endpoint,
        websocketpp::lib::placeholders::_1
    ));
    con->set_message_handler(websocketpp::lib::bind(
        &connection_metadata::on_message,
        metadata_ptr,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
    ));

    m_endpoint.connect(con);

    // Save connection id
    this->con_id = new_id;
    LOG_ONLINETMP("Termina connect");
}

void websocket_endpoint::close(std::string reason, websocketpp::close::status::value code) {
    LOG_ONLINETMP("Empieza close");
    if(this->con_id < 0) return;
    websocketpp::lib::error_code ec;
    
    con_list::iterator metadata_it = m_connection_list.find(this->con_id);
    if (metadata_it == m_connection_list.end()) {
        std::cout << "> No connection found with id " << this->con_id << std::endl;
        return;
    }

    // Check availability
    if(!m_connection_list[con_id]->is_connected()) return;

    // Reset connection id (to indicate we are not connected anymore)
    this->con_id = -1;
    
    m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
    if (ec) {
        std::cout << "> Error initiating close: " << ec.message() << std::endl;
    }
    LOG_ONLINETMP("Termina close");
}

void websocket_endpoint::send(std::string message) {
    LOG_ONLINETMP("Empieza send");
    if(this->con_id < 0) return;
    websocketpp::lib::error_code ec;
    
    con_list::iterator metadata_it = m_connection_list.find(this->con_id);
    if (metadata_it == m_connection_list.end()) {
        std::cout << "> No connection found with id " << this->con_id << std::endl;
        return;
    }

    // Check availability
    if(!m_connection_list[con_id]->is_connected()) return;
    
    m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "> Error sending message: " << ec.message() << std::endl;
        return;
    }
    
    metadata_it->second->record_sent_message(message);
    LOG_ONLINETMP("Termina send");
}

connection_metadata::ptr websocket_endpoint::get_metadata() const {
    LOG_ONLINETMP("Empieza get_metadata");
    if(this->con_id < 0) return connection_metadata::ptr();
    con_list::const_iterator metadata_it = m_connection_list.find(this->con_id);
    LOG_ONLINETMP("Va a returnear algo get_metadata");
    if (metadata_it == m_connection_list.end()) {
        return connection_metadata::ptr();
    } else {
        return metadata_it->second;
    }
}

void websocket_endpoint::setHost(bool newValue) noexcept
{
    host = newValue;
}

bool websocket_endpoint::isHost() const noexcept
{
    return host;
}

bool websocket_endpoint::isOnline() noexcept
{
    return (this->con_id >= 0 && m_connection_list[con_id]->is_connected());
}
