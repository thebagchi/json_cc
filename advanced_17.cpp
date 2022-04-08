#include <iostream>
#include <memory>
#include <utility>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using Allocator = rapidjson::Document::AllocatorType;
using ValuePtr = rapidjson::Value *;

template <typename C, typename T>
struct Props {
  constexpr Props(T C::*member, const char *name, bool required)
      : member{member}, name{name}, required{required} {
    // Empty
  }
  using Type = T;
  T C::*member;
  const char *name;
  bool required;
};

template <typename C, typename T>
constexpr Props<C, T> prop(T C::*member, const char *name,
                           bool required = false) {
  return Props<C, T>{member, name, required};
}

template <typename T, T... S, typename F>
constexpr void for_each(std::integer_sequence<T, S...>, F &&f) {
  (static_cast<void>(f(std::integral_constant<T, S>{})), ...);
}

template <typename T>
void Write(ValuePtr value, Allocator &allocator, T *object) {
  value->SetObject();
  constexpr auto props = std::tuple_size<decltype(T::properties)>::value;
  for_each(std::make_index_sequence<props>{}, [&](const auto &i) {
    constexpr auto property = std::get<i>(T::properties);
    using Type = typename decltype(property)::Type;
    auto key = rapidjson::Value(rapidjson::kStringType);
    key.SetString(property.name, strlen(property.name));
    auto val = rapidjson::Value();
    const auto &ptr = object->*(property.member);
    if (ptr) {
      Write(&val, allocator, ptr.get());
    }
    value->AddMember(key, val, allocator);
  });
}

template <>
void Write<std::string>(ValuePtr value, Allocator &allocator,
                        std::string *object) {
  value->SetString(object->data(), object->length(), allocator);
}

template <>
void Write<std::int64_t>(ValuePtr value, Allocator &allocator,
                         std::int64_t *object) {
  value->SetInt64(*object);
}

template <>
void Write<double>(ValuePtr value, Allocator &allocator, double *object) {
  value->SetDouble(*object);
}

template <>
void Write<bool>(ValuePtr value, Allocator &allocator, bool *object) {
  value->SetBool(*object);
}

using Allocator = rapidjson::Document::AllocatorType;
using ValuePtr = rapidjson::Value *;

class DEF {
 public:
  std::unique_ptr<std::int64_t> long_member_;
  std::unique_ptr<std::string> string_member_;
  std::unique_ptr<bool> bool_member_;
  std::unique_ptr<double> double_member_;

 public:
  constexpr const static auto properties = std::make_tuple(
      prop(&DEF::long_member_, "long"), prop(&DEF::string_member_, "string"),
      prop(&DEF::double_member_, "double"),
      prop(&DEF::bool_member_, "boolean"));
};

class ABC {
 public:
  std::unique_ptr<std::int64_t> long_member_;
  std::unique_ptr<std::string> string_member_;
  std::unique_ptr<bool> bool_member_;
  std::unique_ptr<double> double_member_;
  std::unique_ptr<DEF> def_member_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&ABC::long_member_, "long_value"),
                      prop(&ABC::string_member_, "string_value"),
                      prop(&ABC::double_member_, "double_value"),
                      prop(&ABC::bool_member_, "boolean_value"),
                      prop(&ABC::def_member_, "def_value"));
};

std::string serialize(const rapidjson::Document &document,
                      bool pretty = false) {
  rapidjson::StringBuffer buffer;
  if (!pretty) {
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
  } else {
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
  }
  return buffer.GetString();
}

int main() {
  std::cout << "Running advanced works only with cpp 17 example" << std::endl;
  auto abc = std::make_unique<ABC>();
  abc->string_member_ = std::make_unique<std::string>("hello world");
  abc->def_member_ = std::make_unique<DEF>();
  abc->def_member_->string_member_ = std::make_unique<std::string>("DEF");
  abc->def_member_->double_member_ = std::make_unique<double>(123.45);
  abc->def_member_->long_member_ = std::make_unique<std::int64_t>(12345);
  abc->def_member_->bool_member_ = std::make_unique<bool>(true);

  rapidjson::Document document;
  Write(&document, document.GetAllocator(), abc.get());
  std::cout << serialize(document, true) << std::endl;
}