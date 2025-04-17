//
// Created by 刘智禹 on 25-4-17.
//

#ifndef JSONHELPER_H
#define JSONHELPER_H
#include "json/json.h"
#include "spdlog/spdlog.h"
#include "type_traits"
#include <fstream>

#define CPP_VERSION __cplusplus
#define IS_CPP20 (CPP_VERSION >= 202002L)
#define IS_CPP17 (CPP_VERSION >= 201703L)
#define IS_CPP14 (CPP_VERSION >= 201402L)
#define IS_CPP11 (CPP_VERSION >= 201103L)


#if IS_CPP20
template<typename T>
concept isVector = requires
{
    typename T::value_type; // 检查是否有 value_type
    requires std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type> >;
};
#else
template <typename T>
struct is_std_vector : std::false_type {};

template <typename T, typename A>
struct is_std_vector<std::vector<T, A>> : std::true_type {}; // <type, allocator>

template <typename T>
constexpr bool isVector = is_std_vector<T>::value;
#endif

struct JsonHelper {
    JsonHelper() = default;

    JsonHelper(const Json::Value &json) : config(json) {
    }

    JsonHelper(JsonHelper &&) = default;

    JsonHelper(const JsonHelper &) = default;

    JsonHelper &operator=(const JsonHelper &) = default;

    JsonHelper &operator=(JsonHelper &&) = default;

    ~JsonHelper() = default;

    bool is_json_member(const Json::Value &json, const std::string &key) {
        if (json.isMember(key)) {
            spdlog::info("Json contains key: {}", key);
            return true;
        } else {
            spdlog::warn("Json does not contain key: {}", key);
            return false;
        }
    }

    void config_init(const std::string &file_path) {
        std::ifstream config_file(file_path);
        if (!config_file.is_open()) {
            spdlog::error("fail to open config.json");
            return;
        }
        config_file >> config;
        config_file.close();
    }

    template<typename Container>
    Container read(const std::string &field) {
        using T = typename std::remove_cv<Container>::type;
        T result;

        if (is_json_member(config, field)) {
            if constexpr (isVector<T>) {
                for (const auto &item: config[field]) {
                    using ElemType = typename T::value_type;
                    result.emplace_back(extract_value<ElemType>(item));
                }
            } else {
                result = extract_value<T>(config[field]);
            }
        }

        return result;
    }

private:
    Json::Value config{};

    template<typename T>
    T extract_value(const Json::Value &value) {
        if constexpr (std::is_same_v<T, int>) {
            return value.asInt();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return value.asString();
        } else if constexpr (std::is_same_v<T, double>) {
            return value.asDouble();
        } else if constexpr (std::is_same_v<T, bool>) {
            return value.asBool();
        } else {
            static_assert(!std::is_same_v<T, T>, "Unsupported value type");
        }
    }
};

#endif //JSONHELPER_H
