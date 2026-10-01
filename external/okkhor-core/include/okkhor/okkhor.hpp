#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace okkhor {

class Engine {
public:
    Engine();
    explicit Engine(const std::string& data_dir);

    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&) noexcept;
    Engine& operator=(Engine&&) noexcept;

    std::string transliterate_latin_to_bangla(
        std::string_view input
    ) const;

    std::string transliterate_bangla_to_latin(
        std::string_view input
    ) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace okkhor