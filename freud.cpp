#include <cassert>
#include <iostream>

#include <memory>
#include <tuple>
#include <utility>

namespace detail {
template <std::size_t Index, typename T>
struct get_field_value {
    static const void* call(const T& values, const std::string& name) {
        if (std::get<Index>(values).first == name) {
            return reinterpret_cast<const void*>(
                &std::get<Index>(values).second);
        } else {
            return get_field_value<Index - 1, T>::call(values, name);
        }
    }
};

template <typename T>
struct get_field_value<0, T> {
    static const void* call(const T& values, const std::string& name) {
        if (std::get<0>(values).first == name) {
            return reinterpret_cast<const void*>(&std::get<0>(values).second);
        } else {
            return nullptr;
        }
    }
};

template <typename T, std::size_t Index, typename... Fields>
struct value_pairs
    : value_pairs<T, Index - 1,
                  typename std::tuple_element<Index - 1, T>::type::format,
                  Fields...> {};

template <typename T, typename... Fields>
struct value_pairs<T, 0, Fields...> {
    using type = std::tuple<std::pair<std::string, Fields>...>;
};

template <std::size_t Index, typename T, typename... Pairs>
struct decompose_bytes {
    static typename value_pairs<T, std::tuple_size<T>::value>::type
    call(const char* bytes, const char* position, const T& fields,
         Pairs... pairs) {
        const auto& field = std::get<std::tuple_size<T>::value - Index>(fields);

        // TODO consider alignment
        auto value = *reinterpret_cast<
            const typename std::decay<decltype(field)>::type::format*>(
            position);

        position += sizeof(value);

        return decompose_bytes<
            Index - 1, T, Pairs...,
            std::pair<std::string, const typename std::decay<decltype(
                                       field)>::type::format>>::
            call(bytes, position, fields, pairs...,
                 std::make_pair(field.name, value));
    }
};

template <typename T, typename... Pairs>
struct decompose_bytes<0, T, Pairs...> {
    static std::tuple<Pairs...> call(const char* bytes, const char* position,
                                     const T& fields, Pairs... pairs) {
        return std::make_tuple(pairs...);
    }
};
}

// MemoryObject -------------------------------------------------------
class MemoryObjectBase {
public:
    virtual ~MemoryObjectBase() {}
    virtual const void* get(const std::string& name) const = 0;
};

template <typename T>
class MemoryObjectImpl : public MemoryObjectBase {
public:
    MemoryObjectImpl(const T& fields) : m_field_values{fields} {}
    virtual const void* get(const std::string& name) const {
        return detail::get_field_value<
            std::tuple_size<T>::value - 1,
            decltype(m_field_values)>::call(m_field_values, name);
    }

private:
    T m_field_values;
};

class MemoryObject {
public:
    template <typename T>
    MemoryObject(const T& fields) : m_obj{new MemoryObjectImpl<T>(fields)} {}

    template <typename T>
    const T& get(const std::string& name) const {
        // TODO raise exception on get failure?
        return *reinterpret_cast<const T*>(m_obj->get(name));
    }

private:
    std::unique_ptr<MemoryObjectBase> m_obj;
};

// MemoryObjectMatcher ------------------------------------------------
class MemoryObjectMatcherBase {
public:
    virtual ~MemoryObjectMatcherBase() {}
    virtual std::unique_ptr<MemoryObject> verify(const char* bytes) = 0;
};

template <typename... Fields>
class MemoryObjectMatcherImpl : public MemoryObjectMatcherBase {
public:
    MemoryObjectMatcherImpl(Fields... fields) : fields{fields...} {}

    virtual std::unique_ptr<MemoryObject> verify(const char* bytes) {
        return std::unique_ptr<MemoryObject>{new MemoryObject(
            detail::decompose_bytes<sizeof...(Fields),
                                    decltype(fields)>::call(bytes, bytes,
                                                            fields))};
    }

    std::tuple<Fields...> fields;
};

class MemoryObjectMatcher {
public:
    template <typename... Fields>
    MemoryObjectMatcher(Fields&&... fields)
        : m_obj{new MemoryObjectMatcherImpl<Fields...>{fields...}} {}

    std::unique_ptr<MemoryObject> verify(const char* bytes) {
        return m_obj->verify(bytes);
    }

private:
    std::unique_ptr<MemoryObjectMatcherBase> m_obj;
};

// MemoryFields ------------------------------------------------
template <typename T>
class MemoryField {
public:
    using format = T;

    MemoryField(const std::string& name) : name{name} {}
    std::string name;
};

template <typename T>
class Value : public MemoryField<T> {
public:
    bool verify(const T& t) { return t == val; }
    Value(const std::string& name, const T& val)
        : MemoryField<T>{name}, val{val} {}
    T val;
};

// User facing api
class OpenSSL_SessionMatcher : public MemoryObjectMatcher {
public:
    OpenSSL_SessionMatcher()
        : MemoryObjectMatcher(Value<unsigned int>("ssl_version", 771),
                              Value<unsigned int>("ssl_version2", 771)) {}
};

struct test_obj {
    unsigned int ssl_version;
    unsigned int ssl_version2;
};

int main() {
    const char* bytes = "\x12\x34\x56\x78\x12\x34\x56\x78";
    OpenSSL_SessionMatcher matcher;

    auto obj = matcher.verify(bytes);

    std::cout << obj->get<unsigned int>("ssl_version") << std::endl;
    std::cout << obj->get<unsigned int>("ssl_version2") << std::endl;

    auto test = reinterpret_cast<const test_obj*>(bytes);
    assert(test->ssl_version == obj->get<unsigned int>("ssl_version"));
    assert(test->ssl_version2 == obj->get<unsigned int>("ssl_version2"));
}
