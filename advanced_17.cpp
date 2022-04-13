#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <map>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using Allocator = rapidjson::Document::AllocatorType;
using ValuePtr = rapidjson::Value *;

template<typename C, typename T>
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

template<typename C, typename T>
constexpr Props<C, T> prop(T C::*member, const char *name, bool required = false) {
  return Props<C, T>{member, name, required};
}

template<typename T, T... S, typename F>
constexpr void for_each(std::integer_sequence<T, S...>, F &&f) {
  (static_cast<void>(f(std::integral_constant<T, S>{})), ...);
}

template<typename T>
struct Write {
  void operator()(ValuePtr value, Allocator &allocator, const T *object) {
    value->SetObject();
    constexpr auto props = std::tuple_size<decltype(T::properties)>::value;
    for_each(std::make_index_sequence<props>{}, [&](auto i) {
      constexpr auto property = std::get<i>(T::properties);
      using Type = typename decltype(property)::Type;
      auto key = rapidjson::Value(rapidjson::kStringType);
      key.SetString(property.name, strlen(property.name));
      auto val = rapidjson::Value();
      const auto &ptr = object->*(property.member);
      using P = typename std::remove_reference<decltype(*ptr)>::type;
      if (ptr) {
        Write<P>{}(&val, allocator, ptr.get());
      }
      value->AddMember(key, val, allocator);
    });
  }
};

template<>
struct Write<std::string> {
  void operator()(ValuePtr value, Allocator &allocator, const std::string *object) {
    value->SetString(object->data(), object->length(), allocator);
  }
};

template<>
struct Write<std::int64_t> {
  void operator()(ValuePtr value, Allocator &allocator, const std::int64_t *object) {
    value->SetInt64(*object);
  }
};

template<>
struct Write<double> {
  void operator()(ValuePtr value, Allocator &allocator, const double *object) {
    value->SetDouble(*object);
  }
};

template<>
struct Write<bool> {
  void operator()(ValuePtr value, Allocator &allocator, const bool *object) {
    value->SetBool(*object);
  }
};

template<typename T>
struct Write<std::vector<T>> {
  void operator()(ValuePtr value, Allocator &allocator, const std::vector<T> *object) {
    value->SetArray();
    for (const auto &item : *object) {
      auto temp = rapidjson::Value();
      Write<T>{}(&temp, allocator, &item);
      value->PushBack(temp, allocator);
    }
  }
};

template<typename T>
struct Write<std::map<std::string, T>> {
  void operator()(ValuePtr value, Allocator &allocator, const std::map<std::string, T> *object) {
    value->SetObject();
    for (const auto &item : *object) {
      auto key = rapidjson::Value(item.first.data(), item.first.length(), allocator);
      auto val = rapidjson::Value();
      Write<T>{}(&val, allocator, &item.second);
      value->AddMember(key, val, allocator);
    }
  }
};

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
  std::unique_ptr<std::vector<std::string>> list_member_;
  std::unique_ptr<std::map<std::string, std::string>> dict_member_;

 public:
  constexpr const static auto properties =
      std::make_tuple(prop(&ABC::long_member_, "long_value"),
                      prop(&ABC::string_member_, "string_value"),
                      prop(&ABC::double_member_, "double_value"),
                      prop(&ABC::bool_member_, "boolean_value"),
                      prop(&ABC::def_member_, "def_value"),
                      prop(&ABC::list_member_, "list_value"),
                      prop(&ABC::dict_member_, "dict_value"), true);
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
  abc->list_member_ = std::make_unique<std::vector<std::string>>();
  abc->dict_member_ = std::make_unique<std::map<std::string, std::string>>();
  abc->list_member_->push_back("Hello");
  abc->list_member_->push_back("World");
  abc->dict_member_->insert({"Hello", "World"});

  rapidjson::Document document;
  Write<ABC>{}(&document, document.GetAllocator(), abc.get());
  std::cout << serialize(document, true) << std::endl;
}