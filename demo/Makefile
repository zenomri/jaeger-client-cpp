
send: send_span.cc
	g++ send_span.cc tracer.cc -std=c++17 -lrt -lpthread -lboost_system -lthrift -lyaml-cpp -lopentracing -ljaegertracing -o send
recieve: recieve_span.cc
	g++ recieve_span.cc tracer.cc -std=c++17 -lpthread -lboost_system -lthrift -lyaml-cpp -lopentracing -ljaegertracing -o recieve

