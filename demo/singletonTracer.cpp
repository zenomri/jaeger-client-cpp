#include <iostream>
#include <yaml-cpp/yaml.h>
#include <jaegertracing/Tracer.h>
using namespace std;

/*
* This code examine use of tracer as a singleton
* using the opentracing library
* 
 */

void setUpGlobalTracer(std::string configFilePath)
{
    auto configYAML = YAML::LoadFile(configFilePath);
    auto config = jaegertracing::Config::parse(configYAML);
    auto tracer = jaegertracing::Tracer::make(
        "test-global-tracer", config, jaegertracing::logging::consoleLogger());
    opentracing::Tracer::InitGlobal(
        std::static_pointer_cast<opentracing::Tracer>(tracer));
}

void tracedSubFunction(const std::unique_ptr<opentracing::Span>& parentSpan)
{
    auto span = opentracing::Tracer::Global()->StartSpan(
        "tracedSubroutine", { opentracing::ChildOf(&parentSpan->context()) });
}

void tracedFunction(string trace_name)
{
    auto span = opentracing::Tracer::Global()->StartSpan(trace_name);
    span->SetTag("type","rgw-tracing");
    span->Log({
        {"level","info"},
        {"msg","long log message asdadadadadasdasdasdasds"}
    });
    tracedSubFunction(span);
}




int main(int argc, char* argv[]){
    string config_file = "../demo/config.yml";
    setUpGlobalTracer(config_file);
    vector<string> traces_names;
    vector<thread*> threads;

    string trace_prefix;
    getline(cin, trace_prefix);
    
    int traces_num = 30;
    for(int i = 1; i <= traces_num ; i ++){
        traces_names.push_back(string(trace_prefix + to_string(i)));
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
