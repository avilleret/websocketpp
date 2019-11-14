#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/base64/base64.hpp>

#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class broadcast_server {
public:
    broadcast_server() {
        m_server.init_asio();

        m_server.set_open_handler(bind(&broadcast_server::on_open,this,::_1));
        m_server.set_close_handler(bind(&broadcast_server::on_close,this,::_1));
        m_server.set_message_handler(bind(&broadcast_server::on_message,this,::_1,::_2));

        m_server.clear_access_channels(
              websocketpp::log::alevel::frame_header | websocketpp::log::alevel::frame_payload);

        m_cap_thread = std::thread([&](){
          std::vector<int> compression_params;
          compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
          compression_params.push_back(20);

          cv::Mat small, mat;
          cv::VideoCapture cap;
          std::vector<uchar> jpeg_buf{};

          while(!m_quit)
          {
            if(!cap.isOpened())
              cap.open(0);
            else
            {
              cap >> mat;

              cv::resize(mat, small, cv::Size(), 0.5, 0.5);
              if(cv::imencode(".jpg", small, jpeg_buf, compression_params))
              {
                std::string encoded = websocketpp::base64_encode(jpeg_buf.data(), jpeg_buf.size());

                con_list::iterator it;
                for (it = m_connections.begin(); it != m_connections.end(); ++it) {
                    m_server.send(*it,encoded,websocketpp::frame::opcode::text);
                }
              }
              usleep(30000);
            }
          }
        });
    }

    void on_open(connection_hdl hdl) {
        m_connections.insert(hdl);
    }

    void on_close(connection_hdl hdl) {
        m_connections.erase(hdl);
    }

    void on_message(connection_hdl hdl, server::message_ptr msg) {
        //for (auto it : m_connections) {
        //    m_server.send(it,msg);
        // }
    }

    void run(uint16_t port) {
        m_server.listen(port);
        m_server.start_accept();
        m_server.run();
    }
private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    server m_server;
    con_list m_connections;

    bool m_quit = false;
    std::thread m_cap_thread;
};

int main() {
    broadcast_server server;
    server.run(9003);
}
