#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace internal {
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}

class ABC {
 public:
  std::unique_ptr<std::int64_t> long_member_;
  std::unique_ptr<std::string> string_member_;
  std::unique_ptr<bool> bool_member_;
  std::unique_ptr<double> double_member_;

  ABC() {
    this->long_member_ = internal::make_unique<std::int64_t>(12345);
    this->string_member_ = internal::make_unique<std::string>("abra-ca-dabra");
    this->bool_member_ = internal::make_unique<bool>(false);
    this->double_member_ = internal::make_unique<double>(12.345);
  }
  ABC(const ABC &) = default;
  ABC(ABC &&) = default;
  ABC &operator=(const ABC &) = default;
  virtual ~ABC() = default;

  void clear_long() {
    if (this->long_member_) {
      this->long_member_.reset();
    }
  }

  void clear_bool() {
    if (this->bool_member_) {
      this->bool_member_.reset();
    }
  }

  void clear_string() {
    if (this->string_member_) {
      this->string_member_.reset();
    }
  }

  void clear_double() {
    if (this->double_member_) {
      this->double_member_.reset();
    }
  }

  void set_long(std::int64_t data) {
    if (this->long_member_) {
      *this->long_member_ = data;
    } else {
      this->long_member_ = internal::make_unique<std::int64_t>(data);
    }
  }

  void set_bool(bool data) {
    if (this->bool_member_) {
      *this->bool_member_ = data;
    } else {
      this->bool_member_ = internal::make_unique<bool>(false);
    }
  }

  void set_string(const std::string &data) {
    if (this->string_member_) {
      *this->string_member_ = data;
    } else {
      this->string_member_ = internal::make_unique<std::string>(data);
    }
  }

  void set_double(double data) {
    if (this->double_member_) {
      *this->double_member_ = data;
    } else {
      this->double_member_ = internal::make_unique<double>(data);
    }
  }
};

std::string serialize(const rapidjson::Document &document) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  document.Accept(writer);
  return buffer.GetString();
}

std::string populate_string(const std::string &data) {
  rapidjson::Document document;
  document.SetString(data.data(), data.length());
  return serialize(document);
}

std::string populate_bool(const bool data) {
  rapidjson::Document document;
  document.SetBool(data);
  return serialize(document);
}

std::string populate_null() {
  rapidjson::Document document;
  document.SetNull();
  return serialize(document);
}

std::string populate_long(std::int64_t data) {
  rapidjson::Document document;
  document.SetInt64(data);
  return serialize(document);
}

std::string populate_double(double data) {
  rapidjson::Document document;
  document.SetDouble(data);
  return serialize(document);
}

std::string populate_array(const std::vector<std::string> &array) {
  rapidjson::Document document;
  document.SetArray();
  for (const auto &item : array) {
    rapidjson::Value value;
    value.SetString(item.data(), item.length(), document.GetAllocator());
    document.PushBack(value, document.GetAllocator());
  }
  return serialize(document);
}

std::string populate_dict(const std::map<std::string, std::string> &elements) {
  rapidjson::Document document;
  document.SetObject();
  for (const auto &item : elements) {
    auto key = rapidjson::Value(rapidjson::kStringType);
    auto value = rapidjson::Value(rapidjson::kStringType);
    key.SetString(item.first.data(), item.first.length(),
                  document.GetAllocator());
    value.SetString(item.second.data(), item.second.length(),
                    document.GetAllocator());
    document.AddMember(key, value, document.GetAllocator());
  }
  return serialize(document);
}

std::string populate_struct(const ABC *structure) {
  rapidjson::Document document;
  document.SetObject();
  if (structure->string_member_) {
    const std::string &name = "string_member";
    auto key = rapidjson::Value(rapidjson::kStringType);
    auto value = rapidjson::Value(rapidjson::kStringType);
    key.SetString(name.data(), name.length(), document.GetAllocator());
    value.SetString(structure->string_member_->data(),
                    structure->string_member_->length(),
                    document.GetAllocator());
    document.AddMember(key, value, document.GetAllocator());
  }
  if (structure->bool_member_) {
    const std::string &name = "bool_member";
    auto key = rapidjson::Value(rapidjson::kStringType);
    auto value = rapidjson::Value();
    key.SetString(name.data(), name.length(), document.GetAllocator());
    value.SetBool(*structure->bool_member_);
    document.AddMember(key, value, document.GetAllocator());
  }
  if (structure->long_member_) {
    const std::string &name = "long_member";
    auto key = rapidjson::Value(rapidjson::kStringType);
    auto value = rapidjson::Value();
    key.SetString(name.data(), name.length(), document.GetAllocator());
    value.SetInt64(*structure->long_member_);
    document.AddMember(key, value, document.GetAllocator());
  }
  if (structure->double_member_) {
    const std::string &name = "double_member";
    auto key = rapidjson::Value(rapidjson::kStringType);
    auto value = rapidjson::Value();
    key.SetString(name.data(), name.length(), document.GetAllocator());
    value.SetDouble(*structure->double_member_);
    document.AddMember(key, value, document.GetAllocator());
  }
  return serialize(document);
}

int main() {
  std::cout << populate_null() << std::endl;
  std::cout << populate_string("Hello World") << std::endl;
  std::cout << populate_bool(true) << std::endl;
  std::cout << populate_bool(false) << std::endl;
  std::cout << populate_double(12.345) << std::endl;
  std::cout << populate_long(12345) << std::endl;

  std::cout << populate_array({"l1", "l2", "l3"}) << std::endl;

  std::cout << populate_dict({{"k1", "v1"},
                              {"k2", "v2"},
                              {"k3", "v3"}})
            << std::endl;

  auto abc = internal::make_unique<ABC>();
  std::cout << populate_struct(abc.get()) << std::endl;
  abc->clear_string();
  std::cout << populate_struct(abc.get()) << std::endl;
  return 0;
}
