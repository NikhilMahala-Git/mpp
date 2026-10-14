#include "./string.hpp"
#include "./log.hpp"
#include "./lib.hpp"

MPP_LIB_NAMESPACE_BEGIN

String::String(const char *data) {
    const char *_src_data = data;
    const size_t _src_data_len = mpp::strlen(_src_data);

    char *_dest_data = this->stack_data;
    if (_src_data_len > this->STACK_BUFFER_CAPACITY - 1) {
        this->heap_data_capacity = _src_data_len * 1.5 + 1;
        this->heap_data = (char *)mpp::malloc(sizeof(char) * this->heap_data_capacity);

        _dest_data = this->heap_data;
    }

    mpp::memcpy(_dest_data, _src_data, sizeof(char) * _src_data_len);
    _dest_data[_src_data_len] = 0;

    this->data_length = _src_data_len;
}

String::~String() {
    this->destroy();
}

String::String(const String &other) {
    if (this == &other) {
        return;
    }

    const char *_src_data = other.data();
    const size_t _src_data_len = other.data_length;

    char *_dest_data = this->stack_data;
    if (this->heap_data_capacity != 0 || _src_data_len > this->STACK_BUFFER_CAPACITY - 1) {
        if (this->heap_data_capacity < _src_data_len + 1) {
            this->heap_data_capacity = _src_data_len * 1.5 + 1;
            this->heap_data = (char *)mpp::realloc(this->heap_data, sizeof(char) * this->heap_data_capacity);
        }

        _dest_data = this->heap_data;
    }

    mpp::memcpy(_dest_data, _src_data, sizeof(char) * _src_data_len);
    _dest_data[_src_data_len] = 0;

    this->data_length = _src_data_len;
}

String& String::operator = (const String &other) {
    if (this == &other) {
        return *this;
    }

    const char *_src_data = other.data();
    const size_t _src_data_len = other.data_length;

    char *_dest_data = this->stack_data;
    if (this->heap_data_capacity != 0 || _src_data_len > this->STACK_BUFFER_CAPACITY - 1) {
        if (this->heap_data_capacity < _src_data_len + 1) {
            this->heap_data_capacity = _src_data_len * 1.5;
            this->heap_data = (char *)mpp::realloc(this->heap_data, sizeof(char) * this->heap_data_capacity);
        }

        _dest_data = this->heap_data;
    }

    mpp::memcpy(_dest_data, _src_data, sizeof(char) * _src_data_len);
    _dest_data[_src_data_len] = 0;

    this->data_length = _src_data_len;
    return *this;
}

String::String(String &&other) {
    if (this == &other) {
        return;
    }

    this->destroy();

    this->heap_data = other.heap_data;
    mpp::memcpy(this->stack_data, other.stack_data, sizeof(char) * this->STACK_BUFFER_CAPACITY);

    this->data_length = other.data_length;
    this->heap_data_capacity = other.heap_data_capacity;

    other.heap_data = nullptr;
    other.data_length = 0;
    other.heap_data_capacity = 0;
}

String& String::operator = (String &&other) {
    if (this == &other) {
        return *this;
    }

    this->destroy();

    this->heap_data = other.heap_data;
    mpp::memcpy(this->stack_data, other.stack_data, sizeof(char) * this->STACK_BUFFER_CAPACITY);

    this->data_length = other.data_length;
    this->heap_data_capacity = other.heap_data_capacity;

    other.heap_data = nullptr;
    other.data_length = 0;
    other.heap_data_capacity = 0;

    return *this;
}

const char *String::data() const {
    return this->heap_data_capacity != 0 ? this->heap_data : this->stack_data;
}

size_t String::length() const {
    return this->data_length;
}

void String::reserve(size_t new_capacity) {
    ++new_capacity; // Adding one more for \0

    if (this->heap_data_capacity == 0 && new_capacity > this->STACK_BUFFER_CAPACITY) {
        this->heap_data_capacity = new_capacity;
        this->heap_data = (char *)mpp::malloc(sizeof(char) * this->heap_data_capacity);

        mpp::memcpy(this->heap_data, this->stack_data, this->data_length + 1);
    } else if (this->heap_data_capacity != 0 && new_capacity > this->heap_data_capacity) {
        this->heap_data_capacity = new_capacity;
        this->heap_data = (char *)mpp::realloc(this->heap_data, sizeof(char) * this->heap_data_capacity);
    }
}

void String::clear() {
    (this->heap_data_capacity != 0 ? this->heap_data : this->stack_data)[0] = 0;
    this->data_length = 0;
}

void String::reset() {
    this->destroy();
}

String String::substr(size_t begin, size_t end) const {
    if (begin >= this->data_length || end < begin) {
        MPP_MACRO_ASSERT(false, "Invalid begin, end positions");
    }

    String _substr("");
    _substr.append_at_end(this->data() + begin, (end > this->data_length ? this->data_length : end) - begin);
    return _substr;
}

size_t String::find(const String &substr) const {
    if (this->data_length < substr.data_length) {
        return this->INVALID_POSITION;
    }

    size_t _substr_hash = 0;
    for (size_t i = 0; i < substr.length(); ++i) {
        _substr_hash += substr[i];
    }

    size_t _hash   = 0;
    size_t _offset = 0;

    for (size_t i = 0; i < substr.data_length; ++i) {
        _hash += this->data()[i];
    }

    while (true) {
        if (_hash == _substr_hash) {
            size_t _match = 0;
            for (size_t i = 0; i < substr.data_length; ++i) {
                if (substr[i] == this->data()[_offset + i]) {
                    _match++;
                } else {
                    break;
                }
            }
            if (_match == substr.data_length) {
                return _offset;
            }
        }

        if (_offset == (this->data_length - substr.data_length)) {
            break;
        }
        _hash -= this->data()[_offset];
        _hash += this->data()[_offset + substr.data_length];
        _offset++;
    }

    return this->INVALID_POSITION;
}

bool String::operator == (const String &other) const {
    if (this == &other) {
        return true;
    }

    if (this->data_length != other.data_length) {
        return false;
    }

    size_t _match = 0;
    for (size_t i = 0; i < this->data_length; ++i) {
        if (this->operator[](i) == other[i]) {
            _match++;
        } else {
            break;
        }
    }

    return _match == this->data_length;
}

bool String::operator < (const String &other) const {
    if (this == &other) {
        return false;
    }

    for (size_t i = 0; i < this->data_length; ++i) {
        char _a = this->data()[i];
        char _b = other.data()[i];

        if (_a < _b) {
            return true;
        } else if (_a > _b) {
            return false;
        }
    }

    return this->data_length < other.data_length;
}

bool String::operator > (const String &other) const {
    if (this == &other) {
        return false;
    }

    for (size_t i = 0; i < this->data_length; ++i) {
        char _a = this->data()[i];
        char _b = other.data()[i];

        if (_a > _b) {
            return true;
        } else if (_a < _b) {
            return false;
        }
    }

    return this->data_length > other.data_length;
}

char &String::operator[] (size_t index) {
    MPP_MACRO_ASSERT(index < this->data_length, "Out of bound index access");
    return this->heap_data_capacity != 0 ? *(this->heap_data + index) : *(this->stack_data + index);
}

const char &String::operator[] (size_t index) const {
    MPP_MACRO_ASSERT(index < this->data_length, "Out of bound index access");
    return this->heap_data_capacity != 0 ? *(this->heap_data + index) : *(this->stack_data + index);
}

void String::operator += (const String &other) {
    this->append_at_end(other.data(), other.length());
}

void String::operator += (const char *data) {
    this->append_at_end(data, mpp::strlen(data));
}

void String::operator += (char c) {
    static char _internal_buffer[1];
    _internal_buffer[0] = c;

    this->append_at_end(_internal_buffer, 1);
}

String String::from_int(int64_t v) {
    if (v == INT64_MIN) {
        return "-9223372036854775808";
    }

    String _str("");
    if (v < 0) {
        _str += '-';
        v *= -1;
    }

    while (true) {
        _str += ('0' + (v % 10));
        v /= 10;
        if (v == 0) {
            break;
        }
    }

    for (size_t i = _str[0] == '-' ? 1 : 0, j = _str.length() - 1; i < j; ++i, --j) {
        char _ci = _str[i];
        _str[i] = _str[j];
        _str[j] = _ci;
    }

    return _str;
}

String String::from_uint(uint64_t v) {
    String _str("");

    while (true) {
        _str += ('0' + (v % 10));
        v /= 10;
        if (v == 0) {
            break;
        }
    }

    for (size_t i = 0, j = _str.length() - 1; i < j; ++i, --j) {
        char _ci = _str[i];
        _str[i] = _str[j];
        _str[j] = _ci;
    }

    return _str;
}

void String::destroy() {
    if (this->heap_data_capacity != 0) {
        mpp::free(this->heap_data);
        this->heap_data = nullptr;
        this->heap_data_capacity = 0;
    }
    this->stack_data[0] = 0;

    this->data_length = 0;
}

void String::append_at_end(const char *src_data, size_t src_length) {
    this->reserve(this->data_length + src_length + 1);

    char *_dest_data = (this->heap_data_capacity ? this->heap_data : this->stack_data) + this->data_length;
    mpp::memcpy(_dest_data, src_data, sizeof(char) * src_length);
    _dest_data[src_length] = 0;

    this->data_length += src_length;
}

MPP_LIB_NAMESPACE_END

// TODO: Verify the from_float and from_double methods
#define mpp_custom_assert(...)  MPP_MACRO_ASSERT(__VA_ARGS__)
#define mpp_custom_memcpy(...)  mpp::memcpy(__VA_ARGS__)
#define mpp_custom_memset(...)  mpp::memset(__VA_ARGS__)
#define mpp_custom_memmove(...) mpp::memmove(__VA_ARGS__)
#include "../vendor/ftoa.cpp"

MPP_LIB_NAMESPACE_BEGIN

String String::from_float(float v) {
    String _str("");

    char *_begin = _str.stack_data;
    char *_end   =  xjb_ftoa(v, _begin);

    _str.data_length = (uintptr_t(_end) - uintptr_t(_begin));

    return _str;
}

String String::from_double(double v) {
    String _str("");

    char *_begin = _str.stack_data;
    char *_end   =  xjb_ftoa(v, _begin);

    _str.data_length = (uintptr_t(_end) - uintptr_t(_begin));

    return _str;
}

MPP_LIB_NAMESPACE_END
