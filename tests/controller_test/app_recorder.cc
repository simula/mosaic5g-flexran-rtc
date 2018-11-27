#include <vector>
#include <random>
#include <iostream>

#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
using De = google::protobuf::Descriptor;
using Me = google::protobuf::Message;
using Re = google::protobuf::Reflection;
using Fd = google::protobuf::FieldDescriptor;

#include "catch.hpp"
#include "recorder.h"
namespace alog = flexran::app::log;

void fill_message(Me &m, std::mt19937_64& mt);

TEST_CASE("test binary serialization and deserialization", "[recorder]")
{
  /* I don't want to "invent" all those numbers (and cannot for all), hence the
   * use of a random number generator. To have reproducability, the seed is
   * printed in the beginning so a failing test could be restarted and analyzed */
  std::random_device rd;
  const unsigned int seed = rd();
  std::cout << "test app recorder.cc: seed is " << seed << ", please note if test fails\n";
  std::mt19937_64 mt(seed);
  std::uniform_int_distribution<uint64_t> dist64(0);
  std::uniform_int_distribution<uint8_t> dist8(0);

  /* generate 1000ms of random BS data */
  const unsigned n = dist8(mt) % 10 + 1;
  const uint64_t ms_start = dist64(mt);
  const std::string f = "/tmp/flexran.test.app_recorder.dat";
  alog::job_info info{ms_start, ms_start + n, f, alog::job_type::all};
  std::vector<std::map<uint64_t, alog::bs_dump>> v;
  for (unsigned i = 0; i < n; ++i) {
    protocol::flex_enb_config_reply a;
    fill_message(a, mt);
    protocol::flex_ue_config_reply b;
    fill_message(b, mt);
    protocol::flex_lc_config_reply c;
    fill_message(c, mt);
    std::vector<mac_harq_info_t> harq;
    const unsigned jn = dist8(mt);
    for (unsigned j = 0; j < jn; ++j) {
      std::array<bool, 8> a;
      for (unsigned k = 0; k < 8; ++k) a.at(k) = dist8(mt) < 128;
      protocol::flex_ue_stats_report d;
      fill_message(d, mt);
      harq.push_back(std::make_pair(d, a));
    }
    std::map<uint64_t, alog::bs_dump> m;
    const uint64_t bs_id = dist64(mt);
    m.insert(std::make_pair(bs_id, alog::bs_dump{a, b, c, harq}));
    v.push_back(m);
  }

  //std::string s;
  //google::protobuf::util::JsonPrintOptions opt;
  //opt.add_whitespace = true;
  //google::protobuf::util::MessageToJsonString(v.at(0).begin()->second.enb_config, &s, opt);
  //std::cout << s << "\n";

  alog::recorder::write_binary(info, v);
  std::vector<std::map<uint64_t, alog::bs_dump>> vr = alog::recorder::read_binary(info.filename);
  REQUIRE(v.size() == vr.size());
  REQUIRE(v == vr);
}

void fill_by_type_repeated(Me &m, const Fd *fd, std::mt19937_64& mt)
{
  std::uniform_int_distribution<uint8_t> dist8(0);
  std::uniform_int_distribution<uint32_t> dist32(0);
  std::uniform_int_distribution<uint64_t> dist64(0);
  std::uniform_real_distribution<float> distf(0);
  std::uniform_real_distribution<double> distd(0);
  const int n = dist8(mt) % 5 + 1; // never have 0
  int i = 0;
  const Re *r = m.GetReflection();
  switch (fd->type()) {
  case Fd::Type::TYPE_DOUBLE:
    for (; i < n; ++i) r->AddDouble(&m, fd, distd(mt));
    break;
  case Fd::Type::TYPE_FLOAT:
    for (; i < n; ++i) r->AddFloat(&m, fd, distd(mt));
    break;
  case Fd::Type::TYPE_INT64:
  case Fd::Type::TYPE_SINT64:
  case Fd::Type::TYPE_SFIXED64:
    for (; i < n; ++i) r->AddInt64(&m, fd, dist64(mt));
    break;
  case Fd::Type::TYPE_UINT64:
  case Fd::Type::TYPE_FIXED64:
    for (; i < n; ++i) r->AddUInt64(&m, fd, dist64(mt));
    break;
  case Fd::Type::TYPE_INT32:
  case Fd::Type::TYPE_SINT32:
  case Fd::Type::TYPE_SFIXED32:
    for (; i < n; ++i) r->AddInt32(&m, fd, dist32(mt));
    break;
  case Fd::Type::TYPE_BOOL:
    for (; i < n; ++i) r->AddBool(&m, fd, dist8(mt) % 2);
    break;
  case Fd::Type::TYPE_STRING:
  case Fd::Type::TYPE_BYTES:
    for (; i < n; ++i) r->AddString(&m, fd, std::to_string(dist64(mt)));
    break;
  case Fd::Type::TYPE_UINT32:
  case Fd::Type::TYPE_FIXED32:
    for (; i < n; ++i) r->AddUInt32(&m, fd, dist32(mt));
    break;
  case Fd::Type::TYPE_ENUM:
    for (; i < n; ++i)
      r->AddEnum(&m, fd,
          fd->enum_type()->value(dist8(mt) % fd->enum_type()->value_count())
      );
    break;
  case Fd::Type::TYPE_GROUP:
  case Fd::Type::TYPE_MESSAGE:
    for (; i < n; ++i)
      fill_message(*r->AddMessage(&m, fd), mt);
    break;
  }
}

void fill_by_type(Me &m, const Fd *fd, std::mt19937_64& mt)
{
  std::uniform_int_distribution<uint8_t> dist8(0);
  std::uniform_int_distribution<uint32_t> dist32(0);
  std::uniform_int_distribution<uint64_t> dist64(0);
  std::uniform_real_distribution<float> distf(0);
  std::uniform_real_distribution<double> distd(0);
  const Re *r = m.GetReflection();
  switch (fd->type()) {
  case Fd::Type::TYPE_DOUBLE:
    r->SetDouble(&m, fd, distd(mt));
    break;
  case Fd::Type::TYPE_FLOAT:
    r->SetFloat(&m, fd, distd(mt));
    break;
  case Fd::Type::TYPE_INT64:
  case Fd::Type::TYPE_SFIXED64:
    r->SetInt64(&m, fd, dist64(mt));
    break;
  case Fd::Type::TYPE_UINT64:
  case Fd::Type::TYPE_FIXED64:
  case Fd::Type::TYPE_SINT64:
    r->SetUInt64(&m, fd, dist64(mt));
    break;
  case Fd::Type::TYPE_INT32:
  case Fd::Type::TYPE_SFIXED32:
  case Fd::Type::TYPE_SINT32:
    r->SetInt32(&m, fd, dist32(mt));
    break;
  case Fd::Type::TYPE_BOOL:
    r->SetBool(&m, fd, dist8(mt) % 2);
    break;
  case Fd::Type::TYPE_STRING:
  case Fd::Type::TYPE_BYTES:
    r->SetString(&m, fd, std::to_string(dist64(mt)));
    break;
  case Fd::Type::TYPE_UINT32:
  case Fd::Type::TYPE_FIXED32:
    r->SetUInt32(&m, fd, dist32(mt));
    break;
  case Fd::Type::TYPE_ENUM:
    r->SetEnum(&m, fd,
        fd->enum_type()->value(dist8(mt) % fd->enum_type()->value_count())
    );
    break;
  case Fd::Type::TYPE_GROUP:
  case Fd::Type::TYPE_MESSAGE:
    fill_message(*r->MutableMessage(&m, fd), mt);
    break;
  }
}

/* randomly fills a message with dummy data */
void fill_message(Me &m, std::mt19937_64& mt)
{
  const De *d = m.GetDescriptor();
  for (int i = 0; i < d->field_count(); ++i) {
    const Fd *fd = d->field(i);
    if (!fd) continue;
    if (fd->is_repeated())
      fill_by_type_repeated(m, fd, mt);
    else
      fill_by_type(m, fd, mt);
  }
}
