#include <iostream>

#include <yaml-cpp/yaml.h>

#include <jaegertracing/Tracer.h>
using namespace std;
namespace GlobalTracer{

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

void tracedFunction(string unique_symbol)
{
    auto span = opentracing::Tracer::Global()->StartSpan(string("Trace ").append(unique_symbol));
    span->SetTag("type","rgw-tracing");
    span->Log({
        {"level","info"},
        {"msg","long log message asdadadadadasdasdasdasds"}
    });
    tracedSubFunction(span);
}


}

namespace NonGlobalTracer{

std::shared_ptr<jaegertracing::Tracer> setUpTracer(std::string configFilePath){
    auto configYAML = YAML::LoadFile(configFilePath);
    auto config = jaegertracing::Config::parse(configYAML);
    auto tracer = jaegertracing::Tracer::make(
        "test-non-global-tracer", config, jaegertracing::logging::consoleLogger());
    return std::static_pointer_cast<jaegertracing::Tracer>(tracer);

}

void tracedSubFunction(
                    std::shared_ptr<jaegertracing::Tracer> tracer,
                    const std::unique_ptr<opentracing::Span>& parentSpan)
{
    auto span = tracer->StartSpan(
        "sub span", { opentracing::ChildOf(&parentSpan->context()) });
    span->Finish();
    tracer.get()->close();
}

void tracedFunction(string unique_symbol)
{
    string config_file = "../demo/config.yml";
    auto tracer = setUpTracer(config_file);
    auto span = tracer->StartSpan(string("Trace ").append(unique_symbol));
    span->SetTag("type","rgw-tracing");
    span->Log({
        {"level","info"},
        {"msg","long log message asdadadadadasdasdasdasds"}
    });
    span->Finish();
    tracedSubFunction(tracer, span);
}
}  


namespace {
void jaeger_as_singleton(){
    GlobalTracer::setUpGlobalTracer("../demo/config.yml");
    string trace_name;
    getline(cin, trace_name);
    while(true){
        
        GlobalTracer::tracedFunction(trace_name);
        getline(cin, trace_name);

        //thread t2(GlobalTracer::tracedFunction,second_trace_name);

        //t2.join();
    }
    
    jaegertracing::Tracer::Global()->Close();

}
void jaeger_with_tracer_pointer(){
    while(true){
        string traceName; string second_trace_name;
        getline(cin, traceName);
        getline(cin, second_trace_name);
        thread t1(NonGlobalTracer::tracedFunction, traceName);
        thread t2(NonGlobalTracer::tracedFunction, second_trace_name);

        t1.join();
        t2.join();

   
    }
}
}



int main(int argc, char* argv[]){
    //jaeger_with_tracer_pointer();
    jaeger_as_singleton();

    return 0;
}
