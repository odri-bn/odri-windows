#include "json.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace okkhor::json {
namespace {

class Reader {
public:
  explicit Reader(const std::string &s) : s_(s) {}

  Value parse_value() {
    skip_ws();
    if (at_end())
      fail("unexpected end of input");
    char c = peek();
    switch (c) {
    case '{':
      return parse_object();
    case '[':
      return parse_array();
    case '"': {
      Value v;
      v.v = parse_string();
      return v;
    }
    case 't':
      expect_word("true");
      {
        Value v;
        v.v = true;
        return v;
      }
    case 'f':
      expect_word("false");
      {
        Value v;
        v.v = false;
        return v;
      }
    case 'n':
      expect_word("null");
      {
        Value v;
        v.v = nullptr;
        return v;
      }
    default:
      return parse_number();
    }
  }

  void skip_ws() {
    while (!at_end()) {
      char c = peek();
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        ++i_;
      else
        break;
    }
  }
  bool at_end() const { return i_ >= s_.size(); }

private:
  char peek() const { return s_[i_]; }
  char get() { return s_[i_++]; }
  [[noreturn]] void fail(const std::string &msg) const {
    throw std::runtime_error("JSON parse error at offset " +
                             std::to_string(i_) + ": " + msg);
  }
  void expect(char c) {
    if (at_end() || get() != c)
      fail(std::string("expected '") + c + "'");
  }
  void expect_word(const char *w) {
    for (const char *p = w; *p; ++p) {
      if (at_end() || get() != *p)
        fail(std::string("expected literal ") + w);
    }
  }

  Value parse_object() {
    expect('{');
    Object o;
    skip_ws();
    if (!at_end() && peek() == '}') {
      ++i_;
      Value v;
      v.v = std::move(o);
      return v;
    }
    while (true) {
      skip_ws();
      std::string key = parse_string();
      skip_ws();
      expect(':');
      o.emplace(std::move(key), parse_value());
      skip_ws();
      if (at_end())
        fail("unterminated object");
      char c = get();
      if (c == ',')
        continue;
      if (c == '}')
        break;
      fail("expected ',' or '}'");
    }
    Value v;
    v.v = std::move(o);
    return v;
  }

  Value parse_array() {
    expect('[');
    Array a;
    skip_ws();
    if (!at_end() && peek() == ']') {
      ++i_;
      Value v;
      v.v = std::move(a);
      return v;
    }
    while (true) {
      a.push_back(parse_value());
      skip_ws();
      if (at_end())
        fail("unterminated array");
      char c = get();
      if (c == ',')
        continue;
      if (c == ']')
        break;
      fail("expected ',' or ']'");
    }
    Value v;
    v.v = std::move(a);
    return v;
  }

  static void append_utf8(std::string &out, unsigned cp) {
    if (cp < 0x80) {
      out += static_cast<char>(cp);
    } else if (cp < 0x800) {
      out += static_cast<char>(0xC0 | (cp >> 6));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
      out += static_cast<char>(0xE0 | (cp >> 12));
      out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
      out += static_cast<char>(0xF0 | (cp >> 18));
      out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
      out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    }
  }

  unsigned parse_hex4() {
    unsigned cp = 0;
    for (int n = 0; n < 4; ++n) {
      if (at_end())
        fail("truncated \\u escape");
      char c = get();
      cp <<= 4;
      if (c >= '0' && c <= '9')
        cp |= static_cast<unsigned>(c - '0');
      else if (c >= 'a' && c <= 'f')
        cp |= static_cast<unsigned>(c - 'a' + 10);
      else if (c >= 'A' && c <= 'F')
        cp |= static_cast<unsigned>(c - 'A' + 10);
      else
        fail("bad hex digit in \\u escape");
    }
    return cp;
  }

  std::string parse_string() {
    expect('"');
    std::string out;
    while (true) {
      if (at_end())
        fail("unterminated string");
      char c = get();
      if (c == '"')
        break;
      if (c != '\\') {
        out += c;
        continue;
      }
      if (at_end())
        fail("unterminated escape");
      char e = get();
      switch (e) {
      case '"':
        out += '"';
        break;
      case '\\':
        out += '\\';
        break;
      case '/':
        out += '/';
        break;
      case 'b':
        out += '\b';
        break;
      case 'f':
        out += '\f';
        break;
      case 'n':
        out += '\n';
        break;
      case 'r':
        out += '\r';
        break;
      case 't':
        out += '\t';
        break;
      case 'u': {
        unsigned cp = parse_hex4();
        if (cp >= 0xD800 && cp <= 0xDBFF && i_ + 1 < s_.size() &&
            s_[i_] == '\\' && s_[i_ + 1] == 'u') {
          i_ += 2;
          unsigned lo = parse_hex4();
          if (lo >= 0xDC00 && lo <= 0xDFFF)
            cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
          else
            append_utf8(out, cp), cp = lo;
        }
        append_utf8(out, cp);
        break;
      }
      default:
        fail("unknown escape");
      }
    }
    return out;
  }

  Value parse_number() {
    std::size_t start = i_;
    if (!at_end() && (peek() == '-' || peek() == '+'))
      ++i_;
    bool digits = false;
    while (!at_end()) {
      char c = peek();
      if ((c >= '0' && c <= '9')) {
        digits = true;
        ++i_;
      } else if (c == '.' || c == 'e' || c == 'E' || c == '-' || c == '+')
        ++i_;
      else
        break;
    }
    if (!digits)
      fail("invalid value");
    Value v;
    v.v = std::stod(s_.substr(start, i_ - start));
    return v;
  }

  const std::string &s_;
  std::size_t i_ = 0;
};

} // namespace

Value parse(const std::string &text) {
  Reader r(text);
  Value v = r.parse_value();
  r.skip_ws();
  if (!r.at_end())
    throw std::runtime_error("JSON parse error: trailing content");
  return v;
}

Value parse_file(const std::string &path) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    throw std::runtime_error("cannot open data file: " + path);
  std::ostringstream ss;
  ss << in.rdbuf();
  try {
    return parse(ss.str());
  } catch (const std::exception &e) {
    throw std::runtime_error(path + ": " + e.what());
  }
}

} // namespace okkhor::json
