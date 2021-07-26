#include "tracer.h"

uint64_t buffTo64int(char * buffer)
{ 
  unsigned char* unsigned_buff = (unsigned char*)buffer;
  auto value = 
  static_cast<uint64_t>(unsigned_buff[0]) |
  static_cast<uint64_t>(unsigned_buff[1]) << 8 |
  static_cast<uint64_t>(unsigned_buff[2]) << 16 |
  static_cast<uint64_t>(unsigned_buff[3]) << 24 |
  static_cast<uint64_t>(unsigned_buff[4]) << 32 |
  static_cast<uint64_t>(unsigned_buff[5]) << 40 |
  static_cast<uint64_t>(unsigned_buff[6]) << 48 |
  static_cast<uint64_t>(unsigned_buff[7]) << 56;
  return value;
}

uint32_t buffto32int(char * buffer)
{
  unsigned char* unsigned_buff = (unsigned char*)buffer;
    uint32_t value = 
  static_cast<uint32_t>(unsigned_buff[0]) |
  static_cast<uint32_t>(unsigned_buff[1]) << 8 |
  static_cast<uint32_t>(unsigned_buff[2]) << 16 |
  static_cast<uint32_t>(unsigned_buff[3]) << 24;
  return value;
}

std::unique_ptr<jaegertracing::SpanContext> custom_extract(std::istream& stream) {
  
  char buff[37];//minimum size of SpanContext (without baggage)
  char* data = (char*)buff; 
  stream.read(data, 37);

  const uint64_t traceIDHigh = 0;
  const uint64_t traceIDLow = buffTo64int(data+8);
  jaegertracing::TraceID traceID(traceIDHigh, traceIDLow);
  const uint64_t spanID = buffTo64int(data+16);
  const uint64_t parentID = buffTo64int(data+24);
  unsigned ch = '\0';
  ch = *(data+32);
  const auto flags = static_cast<unsigned char>(ch);

  const uint32_t numBaggageItems = buffto32int(data+33);
  jaegertracing::propagation::BinaryPropagator::StrMap baggage;
  baggage.reserve(numBaggageItems);
  jaegertracing::SpanContext ctx(traceID, spanID, parentID, flags, baggage);
  return std::unique_ptr<jaegertracing::SpanContext>(new jaegertracing::SpanContext(ctx));

}