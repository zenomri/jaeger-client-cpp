#include <boost/asio.hpp>
#include <iostream>
#include <vector>

#include "tracer.h"
using namespace std;
int main ()
{
  const auto tracer = jaegertracing::Tracer::make("test-inject-extract", jaeger_configuration::jaeger_default_config, jaegertracing::logging::consoleLogger());
  opentracing::Tracer::InitGlobal(std::static_pointer_cast<opentracing::Tracer>(tracer));
  const auto span = opentracing::Tracer::Global()->StartSpan("inject span");
   span->Finish();
while(true){
   boost::asio::io_service io_service;
   boost::asio::ip::tcp::acceptor a(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 12346));
   boost::asio::ip::tcp::socket sock(io_service);
   a.accept(sock);
  
   boost::asio::streambuf request;
   std::ostream request_stream(&request);
   opentracing::Tracer::Global()->Inject(span->context(), request_stream);
   auto bytes_Transfered = boost::asio::write(sock, request);
   cout << "bytes transmitted " << bytes_Transfered << endl;

}
   // Send the message.
   //boost::asio::write(tcp_socket, request);

   return 0;
}