#ifndef _MPP_LIB_STRING_HPP_
#define _MPP_LIB_STRING_HPP_
#include "./defs.hpp"

// TODO
// - String comparision operators

MPP_LIB_NAMESPACE_BEGIN

class String {
public:
    String(const char *data);
    ~String();

    String(const String &other);
    String& operator = (const String &other);

    String(String &&other);
    String& operator = (String &&other);

public:
    // Data is null-terminated.
    const char *data() const;
    size_t length() const;

    inline bool empty() { return this->data_length == 0; }

    // @note capacity includes null-terminator
    inline size_t capacity() { return this->heap_data_capacity != 0 ? this->heap_data_capacity : this->STACK_BUFFER_CAPACITY; }

    // @note: if new_capacity <= internal capacity then call is ignored.
    // @note: It is not mandatory that after this call capacity will exactly become new_capacity
    //        It will become >= new_capacity
    void reserve(size_t new_capacity);

    // @note: Does not resets the internal capacity. For that use reset.
    void clear();

    // @note: Clears and resets the internal capacity
    void reset();

public:
    // @note: end position is not included.
    // @note: end position is clamped.
    String substr(size_t begin, size_t end = INVALID_POSITION) const;

    // @note: Best time complexity  O(N + M)
    // @note: Worst time complexity O(N * M)
    // @returns: String::INVALID_POSITION if not found
    size_t find(const String &substr) const;

    bool operator == (const String &other) const;
    bool operator < (const String &other) const;
    bool operator > (const String &other) const;

    inline bool operator <= (const String &other) { return (this->operator < (other) || this->operator == (other)); }
    inline bool operator >= (const String &other) { return (this->operator > (other) || this->operator == (other)); }

public:
    // Aborts the program on out of bound access.
    char &operator[] (size_t index);
    // Aborts the program on out of bound access.
    const char &operator[] (size_t index) const;

#define DECLARE_PLUS_EQUAL_AND_IMPLEMENT_PLUS_OPERATOR(operand_type, operand_name)      \
    void operator += (operand_type operand_name);                                       \
    inline String operator + (operand_type operand_name) const {                        \
        String _copy = *this; _copy += operand_name;                                    \
        return _copy;                                                                   \
    }

    DECLARE_PLUS_EQUAL_AND_IMPLEMENT_PLUS_OPERATOR(const String &, other);
    DECLARE_PLUS_EQUAL_AND_IMPLEMENT_PLUS_OPERATOR(const char *, data);
    DECLARE_PLUS_EQUAL_AND_IMPLEMENT_PLUS_OPERATOR(char, c);

#undef DECLARE_PLUS_EQUAL_AND_IMPLEMENT_PLUS_OPERATOR

public:
    static String from_int(int64_t v);
    static String from_uint(uint64_t v);

    static String from_float(float v);
    static String from_double(double v);

private:
    void destroy();
    void append_at_end(const char *src_data, size_t src_length);

private:
    static constexpr size_t MAX_LENGTH = SIZE_MAX - 1;
    static constexpr size_t INVALID_POSITION = SIZE_MAX;

private:
    static constexpr size_t STACK_BUFFER_CAPACITY = 256;

private:
    char *heap_data = nullptr;
    char  stack_data[STACK_BUFFER_CAPACITY] = { 0 };

    size_t data_length = 0;
    size_t heap_data_capacity = 0;
};

MPP_LIB_NAMESPACE_END

#endif // _MPP_LIB_STRING_HPP_
