#ifndef _TRACE_WRAPPER
#define _TRACE_WRAPPER

#include <iostream>
#include <yaml-cpp/yaml.h>
#include <jaegertracing/Tracer.h>

using namespace std;

static bool g_disable_tracing = false;
static auto noop_tracer = opentracing::MakeNoopTracer();

void signal_handler(int signal)
{
  if (signal == SIGUSR1){
    g_disable_tracing = true;
    cout << "Disabling tracers" << endl;

  }
  else if (signal == SIGUSR2) {
    g_disable_tracing = false;
    cout << "Enabling tracers" << endl;

  }
  
}
class TracerWrapper {
private:
    std::shared_ptr<opentracing::Tracer> tracer;


    std::shared_ptr<opentracing::Tracer> setUpTracer(std::string configFilePath){
        auto configYAML = YAML::LoadFile(configFilePath);
        auto config = jaegertracing::Config::parse(configYAML);
        auto tracer = jaegertracing::Tracer::make(
            "test-disabling-tracers", config, jaegertracing::logging::consoleLogger());
        return std::static_pointer_cast<opentracing::Tracer>(tracer);
    }
public:
  TracerWrapper(string config_file){
      tracer = setUpTracer(config_file);

  }

  std::unique_ptr<opentracing::Span> StartSpan(
      opentracing::string_view operation_name,
      std::initializer_list<opentracing::option_wrapper<opentracing::StartSpanOption>> option_list = {})
      {
          if(!g_disable_tracing)
            return tracer->StartSpan(operation_name, option_list);
          return noop_tracer->StartSpan(operation_name,option_list);
      }

  ~TracerWrapper(){
        tracer->Close();
      }
    
};
#endif