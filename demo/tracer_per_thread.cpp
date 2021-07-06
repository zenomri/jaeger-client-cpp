#include <iostream>
#include <yaml-cpp/yaml.h>
#include <jaegertracing/Tracer.h>
#define TRACES_NUM 10
using namespace std;

/*
* This code examine use of multiple tracers, tracer per thread.
* 
* 
 */


std::shared_ptr<opentracing::Tracer> setUpTracer(std::string configFilePath){
    auto configYAML = YAML::LoadFile(configFilePath);
    auto config = jaegertracing::Config::parse(configYAML);
    auto tracer = jaegertracing::Tracer::make(
        "test-non-global-tracer3", config, jaegertracing::logging::consoleLogger());
    return std::static_pointer_cast<opentracing::Tracer>(tracer);

}

void tracedSubFunction(
                    std::shared_ptr<opentracing::Tracer> tracer,
                    const std::unique_ptr<opentracing::Span>& parentSpan)
{
    auto span = tracer->StartSpan(
        "sub span", { opentracing::ChildOf(&parentSpan->context()) });
    span->Finish();
    tracer.get()->Close();
}

void tracedFunction(
                    string span_name)
{
    string config_file = "../demo/config.yml";
    auto tracer = setUpTracer(config_file);
    auto span = tracer->StartSpan(span_name);
    span->SetTag("type","rgw-tracing");
    span->Log({
        {"level","info"},
        {"msg","long log message asdadadadadasdasdasdasds"}
    });
    span->Finish();
    tracedSubFunction(tracer, span);
}

int main(int argc, char* argv[]){
    
    vector<string> traces_names;
    vector<thread*> threads;

    string traces_prefix;
    getline(cin, traces_prefix);
    
    while(true){
        for(int i = 1; i <= TRACES_NUM ; i ++){
            traces_names.push_back(string(traces_prefix + to_string(i)));
        }
    
        for(string name : traces_names){
            thread* t1 = new thread(tracedFunction,name);
            threads.push_back(t1);

        }
        for(thread* t : threads){
            t->join();
            delete t;
        }
        getline(cin, traces_prefix);
    }
    

    return 0;
}
