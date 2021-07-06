
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <jaegertracing/Tracer.h>
#include "TracerWrapper.cpp"
using namespace std;
#define TRACERS_NUM 3

/*
* This code examine use of Tracer Wrapper, which helps us disabling/enabling tracer,
* without performing operations on the tracer and spans
* using global No-Op tracer and boolean variable.
 */


void tracedSubFunction(
                    TracerWrapper& tracer,
                    const std::unique_ptr<opentracing::Span>& parentSpan)
{
    string span_name = parentSpan.get()->BaggageItem("traceName");
    span_name.append(" child");
    auto span = tracer.StartSpan(span_name, { opentracing::ChildOf(&parentSpan->context()) });
    span->Finish();
    
}

void tracedFunction(string trace_name){

  //init Tracer
  string config_file = "../demo/config.yml";
  TracerWrapper t_wrapper(config_file);

  //Start Span
  auto span = t_wrapper.StartSpan(trace_name);
  span->SetTag("type","rgw-tracing");
  span->Log({
      {"level","info"},
      {"msg","long log message asdadadadadasdasdasdasds"}
  });
  span->SetBaggageItem("traceName",trace_name);
  span->Finish();

  //creating sub-span in infinite loop
  while(true){
    tracedSubFunction(t_wrapper, span);
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
    
    

}


int main(int argc, char* argv[]){
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    vector<string> traces_names;
    vector<thread*> threads;

    string trace_name;
    getline(cin, trace_name);

    for(int i = 1; i <= TRACERS_NUM ; i ++){
        traces_names.push_back(string(trace_name + to_string(i)));
    }
  

    for(string name : traces_names){
        thread* t1 = new thread(tracedFunction,name);
        threads.push_back(t1);

    }
    for(thread* t : threads){
        t->join();
        delete t;
    }
    return 0;
}
