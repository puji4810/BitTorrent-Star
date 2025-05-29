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
template <typename T>
concept isVector = requires {
    typename T::value_type; // 检查是否有 value_type
    requires std::same_as<T, std::vector<typename T::value_type, typename T::allocator_type>>;
};
#else
template <typename T>
struct is_std_vector : std::false_type
{
};

template <typename T, typename A>
struct is_std_vector<std::vector<T, A>> : std::true_type
{
}; // <type, allocator>

template <typename T>
constexpr bool isVector = is_std_vector<T>::value;
#endif

struct JsonHelper
{
    JsonHelper() = default;

    JsonHelper(const Json::Value &json) : config(json)
    {
    }

    JsonHelper(JsonHelper &&) = default;

    JsonHelper(const JsonHelper &) = default;

    JsonHelper &operator=(const JsonHelper &) = default;

    JsonHelper &operator=(JsonHelper &&) = default;

    ~JsonHelper() = default;

    bool is_json_member(const Json::Value &json, const std::string &key) const
    {
        if (json.isMember(key))
        {
            spdlog::debug("Json contains key: {}", key);
            return true;
        }
        else
        {
            spdlog::warn("Json does not contain key: {}", key);
            return false;
        }
    }

    void config_init(const std::string &file_path)
    {
        std::ifstream config_file(file_path);
        if (!config_file.is_open())
        {
            spdlog::error("fail to open config.json");
            return;
        }
        config_file >> config;
        config_file.close();
    }

    const Json::Value &get_config() const
    {
        return config;
    }

    Json::Value get_nested_value(const std::string &field) const
    {
        Json::Value current = config;
        std::string curr_field;
        std::istringstream ss(field);

        while (std::getline(ss, curr_field, '.'))
        {
            if (current.isObject() && current.isMember(curr_field))
            {
                current = current[curr_field];
            }
            else
            {
                spdlog::warn("Field '{}' not found in path '{}'", curr_field, field);
                return Json::Value();
            }
        }

        spdlog::debug("Found value for '{}': {}", field, current.toStyledString());
        return current;
    }

    template <typename Container>
    Container read(const std::string &field) const
    {
        using T = typename std::remove_cv<Container>::type;
        T result{};

        if (field.find('.') != std::string::npos)
        {
            Json::Value nested_value = get_nested_value(field);
            if (!nested_value.isNull())
            {
                if constexpr (isVector<T>)
                {
                    for (const auto &item : nested_value)
                    {
                        using ElemType = typename T::value_type;
                        result.emplace_back(extract_value<ElemType>(item));
                    }
                }
                else
                {
                    result = extract_value<T>(nested_value);
                }
                spdlog::debug("Read nested value '{}': {}", field, nested_value.toStyledString());
            }
        }
        else if (is_json_member(config, field))
        {
            if constexpr (isVector<T>)
            {
                for (const auto &item : config[field])
                {
                    using ElemType = typename T::value_type;
                    result.emplace_back(extract_value<ElemType>(item));
                }
            }
            else
            {
                result = extract_value<T>(config[field]);
            }
        }

        return result;
    }

private:
    Json::Value config{};

    template <typename T>
    T extract_value(const Json::Value &value) const
    {
        if constexpr (std::is_same_v<T, int>)
        {
            return value.asInt();
        }
        else if constexpr (std::is_same_v<T, std::uint32_t>)
        {
            return value.asUInt();
        }
        else if constexpr (std::is_same_v<T, long>)
        {
            return value.asLargestInt();
        }
        else if constexpr (std::is_same_v<T, unsigned long>)
        {
            return value.asLargestUInt();
        }
        else if constexpr (std::is_same_v<T, Json::Int>)
        {
            return value.asInt64();
        }
        else if constexpr (std::is_same_v<T, Json::UInt>)
        {
            return value.asUInt64();
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return value.asString();
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return value.asDouble();
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return value.asBool();
        }
        else
        {
            static_assert(!std::is_same_v<T, T>, "Unsupported value type");
        }
    }
};

#endif // JSONHELPER_H
