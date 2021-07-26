// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#ifndef TRACER_WRAPPER_H
#define TRACER_WRAPPER_H

#define SIGNED_RIGHT_SHIFT_IS 1
#define ARITHMETIC_RIGHT_SHIFT 1
#include <jaegertracing/Tracer.h>


namespace tracing {

  class Span {
    private:
      std::unique_ptr<opentracing::Span> span;

    public:
      Span(std::unique_ptr<opentracing::Span>& _span);
      ~Span();
      // Returns SpanReference of this span, which needed to create child span
      opentracing::SpanReference get_reference();
      // Gets the value for a baggage item given its key. 
      // Returns the empty string if the value isn't found in this Span.
      std::string get_baggage(const std::string& key);
      // Adds a tag to the span.
      // If there is a pre-existing tag set for `key`, it is overwritten.
      // Tags could be used for searching traces.
      void set_tag(const std::string& key, const std::string& value);
      // Sets a key:value pair on this Span, to record logging data.
      void set_log(const std::string& key, const std::string& value);
      // Sets a key:value pair on this Span, that also propagates to child spans.
      void set_baggage(const std::string& key, const std::string& value);  
    };

  class Tracer {
    private:
      const static std::shared_ptr<opentracing::Tracer> noop_tracer;
      std::shared_ptr<opentracing::Tracer> open_tracer;
    public:
      Tracer(jaegertracing::Config& conf);
      Tracer(const std::string& config_file_path);
      ~Tracer();

      bool is_enabled() const;
      // creates and returns a new span with `trace_name` 
      // this span represents a trace, since it has no parent.
      std::unique_ptr<Span> start_trace(const std::string& trace_name);
      // creates and returns a new span with `trace_name` which parent span is `parent_span'
      std::unique_ptr<Span> start_span(const std::string& span_name, std::unique_ptr<tracing::Span>& parent_span);
  };
} // namespace tracing

namespace jaeger_configuration {

  extern jaegertracing::Config jaeger_default_config;
}// namespace jaeger_configuration

#endif // TRACER_WRAPPER_H
