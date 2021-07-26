#include <boost/asio.hpp>
#include <thread>
#include <iostream>
#include <vector>
#include "tracer.h"
#include "utils.h"
using namespace std;
using namespace boost::asio;
using namespace opentracing;
using ip::tcp;


int main() {

  const auto tracer = jaegertracing::Tracer::make("test-inject-extract", jaeger_configuration::jaeger_default_config, jaegertracing::logging::consoleLogger());
  opentracing::Tracer::InitGlobal(std::static_pointer_cast<opentracing::Tracer>(tracer));
  boost::asio::io_service io_service;
  //socket creation
  tcp::socket socket(io_service);
  //connection
  socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 12346 ));
  // request/message from client
  boost::system::error_code error;

  // getting response from server
  boost::asio::streambuf receive_buffer;
  std::istream request_stream(&receive_buffer);
  auto transfered_bytes = boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
  if( error && error != boost::asio::error::eof ) {
    cout << "receive failed: " << error.message() << endl;
    return 1;
  }
  //get span_context from the stream
  auto parent_span = custom_extract(request_stream);
  //send new span, with parent span
  auto span = opentracing::Tracer::Global()->StartSpan("extracted Span 5", { opentracing::ChildOf(parent_span.release()) }).release();
  span->Finish();
  opentracing::Tracer::Global()->Close();
  return 0;
}