#include <iostream>
#include <memory>
#include <utility>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

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
constexpr Props<C, T> prop(T C::*member, const char *name,
                           bool required = false) {
  return Props<C, T>{member, name, required};
}

template<typename T>
int tuple_size(T tuple) {
  return std::tuple_size<T>{};
}

using Allocator = rapidjson::Document::AllocatorType;
using ValuePtr = rapidjson::Value *;
template<typename T>
void Write(ValuePtr value, Allocator &allocator, T *object) {
  value->SetObject();
  Write(T::properties(), allocator, object, value);
}

template<>
void Write<std::string>(ValuePtr value, Allocator &allocator,
                        std::string *object) {
  value->SetString(object->data(), object->length(), allocator);
}

template<>
void Write<std::int64_t>(ValuePtr value, Allocator &allocator,
                         std::int64_t *object) {
  value->SetInt64(*object);
}

template<>
void Write<double>(ValuePtr value, Allocator &allocator, double *object) {
  value->SetDouble(*object);
}

template<>
void Write<bool>(ValuePtr value, Allocator &allocator, bool *object) {
  value->SetBool(*object);
}

template<typename T, std::size_t N, typename O>
struct W {
  static void E(const T &t, Allocator &a, O *o, ValuePtr v) {
    constexpr size_t R = N - 1;
    auto props = std::get<std::tuple_size<T>::value - N>(t);
    auto key = rapidjson::Value(rapidjson::kStringType);
    key.SetString(props.name, strlen(props.name));
    auto val = rapidjson::Value();
    const auto &ptr = o->*(props.member);
    if (ptr) {
      Write(&val, a, ptr.get());
    }
    v->AddMember(key, val, a);
    W<T, R, O>::E(t, a, o, v);
  }
};

template<typename T, typename O>
struct W<T, 0, O> {
  static void E(const T &t, Allocator &a, O *o, ValuePtr v) {
    // EMPTY
  }
};

template<typename T, typename O>
void Write(const T &t, Allocator &a, O *o, ValuePtr v) {
  constexpr size_t N = std::tuple_size<T>::value;
  W<T, N, O>::E(t, a, o, v);
}

class DEF {
 public:
  std::unique_ptr<std::int64_t> long_member_;
  std::unique_ptr<std::string> string_member_;
  std::unique_ptr<bool> bool_member_;
  std::unique_ptr<double> double_member_;

 public:
  static std::tuple<Props<DEF, std::unique_ptr<std::int64_t>>,
                    Props<DEF, std::unique_ptr<std::string>>,
                    Props<DEF, std::unique_ptr<double>>,
                    Props<DEF, std::unique_ptr<bool>>>
  properties() {
    return std::make_tuple(prop(&DEF::long_member_, "long"),
                           prop(&DEF::string_member_, "string"),
                           prop(&DEF::double_member_, "double"),
                           prop(&DEF::bool_member_, "boolean"));
  }
};

class ABC {
 public:
  std::unique_ptr<std::int64_t> long_member_;
  std::unique_ptr<std::string> string_member_;
  std::unique_ptr<bool> bool_member_;
  std::unique_ptr<double> double_member_;
  std::unique_ptr<DEF> def_member_;

 public:
  static std::tuple<Props<ABC, std::unique_ptr<std::int64_t>>,
                    Props<ABC, std::unique_ptr<std::string>>,
                    Props<ABC, std::unique_ptr<double>>,
                    Props<ABC, std::unique_ptr<bool>>,
                    Props<ABC, std::unique_ptr<DEF>>>
  properties() {
    return std::make_tuple(prop(&ABC::long_member_, "long_value"),
                           prop(&ABC::string_member_, "string_value"),
                           prop(&ABC::double_member_, "double_value"),
                           prop(&ABC::bool_member_, "boolean_value"),
                           prop(&ABC::def_member_, "def_value"));
  }
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
  std::cout << "Running advanced example" << std::endl;
  std::cout << tuple_size(ABC::properties()) << std::endl;
  auto abc = make_unique<ABC>();
  abc->string_member_ = make_unique<std::string>("hello world");
  abc->def_member_ = make_unique<DEF>();
  abc->def_member_->string_member_ = make_unique<std::string>("DEF");
  abc->def_member_->double_member_ = make_unique<double>(123.45);
  abc->def_member_->long_member_ = make_unique<std::int64_t>(12345);
  abc->def_member_->bool_member_ = make_unique<bool>(true);

  rapidjson::Document document;
  Write(&document, document.GetAllocator(), abc.get());
  std::cout << serialize(document) << std::endl;
  std::cout << serialize(document, true) << std::endl;
}
