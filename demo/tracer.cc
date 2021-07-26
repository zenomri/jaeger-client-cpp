// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab ft=cpp

#include "tracer.h"
#include <yaml-cpp/yaml.h>

namespace tracing {

  const std::shared_ptr<opentracing::Tracer> Tracer::noop_tracer = opentracing::MakeNoopTracer();

  Tracer::Tracer(jaegertracing::Config& conf):open_tracer(jaegertracing::Tracer::make(conf)) {};

  Tracer::Tracer(const std::string& config_file_path) {
    auto config_yaml = YAML::LoadFile(config_file_path);
    auto config = jaegertracing::Config::parse(config_yaml);
    open_tracer =  jaegertracing::Tracer::make(config);
  };

  Tracer::~Tracer() {open_tracer->Close();}

  std::unique_ptr<Span> Tracer::start_trace(const std::string& trace_name) {
    std::unique_ptr<opentracing::Span> span;
    if(is_enabled()) {
      span = open_tracer->StartSpan(trace_name);
    }
    else {
      span = noop_tracer->StartSpan(trace_name);
    }
    return std::unique_ptr<Span>(new Span(span));
  }

  std::unique_ptr<Span> Tracer::start_span(const std::string& span_name, std::unique_ptr<Span>& parent_span) {
    std::unique_ptr<opentracing::Span> span;
    if(is_enabled()) {
      span = open_tracer->StartSpan(span_name, {parent_span->get_reference()});
    }
    else {
      span = noop_tracer->StartSpan(span_name);
    }
    return std::unique_ptr<Span>(new Span(span));
  }

  bool Tracer::is_enabled() const {
    return true;
  }

  Span::Span(std::unique_ptr<opentracing::Span>& _span):span(_span.release()) {};

  Span::~Span() {span->Finish();}

  std::string Span::get_baggage(const std::string& key) {
    return span->BaggageItem(key);
  }

  opentracing::SpanReference Span::get_reference() {
    return opentracing::ChildOf(&span->context());
  }

  void Span::set_tag(const std::string& key, const std::string& value) {
    span->SetTag(key, value);
  }

  void Span::set_log(const std::string& key, const std::string& value) {
    span->Log({{key, value}});
  }

  void Span::set_baggage(const std::string& key, const std::string& value) {
    span->SetBaggageItem(key, value);
  }

}

namespace jaeger_configuration {

   jaegertracing::samplers::Config const_sampler("const", 1, "", 0, jaegertracing::samplers::Config::defaultSamplingRefreshInterval());

   jaegertracing::reporters::Config reporter_default_config(jaegertracing::reporters::Config::kDefaultQueueSize, jaegertracing::reporters::Config::defaultBufferFlushInterval(), true, jaegertracing::reporters::Config::kDefaultLocalAgentHostPort, "");

   jaegertracing::propagation::HeadersConfig headers_config("","","","");

   jaegertracing::baggage::RestrictionsConfig baggage_config(false, "", std::chrono::steady_clock::duration());

   jaegertracing::Config jaeger_default_config(false, const_sampler, reporter_default_config, headers_config, baggage_config, "RGW-Tracing", std::vector<jaegertracing::Tag>());
}
