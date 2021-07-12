#ifndef _TRACE_WRAPPER
#define _TRACE_WRAPPER

#include <iostream>
#include <jaegertracing/Tracer.h>

#define Sampler jaegertracing::samplers
#define Reporter jaegertracing::reporters
#define Propagation jaegertracing::propagation
#define Baggage jaegertracing::baggage


using namespace std;
using Span = opentracing::Span;
using Tracer = opentracing::Tracer;
using Tag = jaegertracing::Tag;
using Clock = std::chrono::steady_clock;

static bool g_disable_tracing = false;
static auto noop_tracer      = opentracing::MakeNoopTracer();
Sampler::Config sampler_config("const",
                                      1,        /* param */ 
                                      "",       /* SamplingServerURL */
                                      0,        /* MaxOperations */
                                      Sampler::Config::defaultSamplingRefreshInterval() /* SamplingRefreshInterval */);



Reporter::Config reporter_config(
                                Reporter::Config::kDefaultQueueSize,
                                Reporter::Config::defaultBufferFlushInterval(),
                                true /* log spans to a given logger*/,
                                Reporter::Config::kDefaultLocalAgentHostPort,
                                "" /*  Default endpoint () */);
Propagation::HeadersConfig headers_config("","","","");
Baggage::RestrictionsConfig baggage_config(false, "", Clock::duration());
jaegertracing::Config config(false /* disabled */,
                            false /* traceId128Bit */,
                            sampler_config,
                            reporter_config,
                            headers_config,
                            baggage_config,
                            "Service Name",
                            std::vector<Tag>() /* tags */,
                            jaegertracing::propagation::Format::JAEGER /* encoding on wire */);

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
    std::shared_ptr<Tracer> tracer;


public:
  TracerWrapper(string service_name){

      this->tracer = jaegertracing::Tracer::make(config);

  }

  std::unique_ptr<Span> StartSpan(
      std::string operation_name,
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