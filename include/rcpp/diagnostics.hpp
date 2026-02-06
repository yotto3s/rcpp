// diagnostics.hpp - Error formatting via reflection
// Part of the C++26 Refinement Types Library

#ifndef RCPP_DIAGNOSTICS_HPP
#define RCPP_DIAGNOSTICS_HPP

#include <format>
#include <string>
#include <string_view>
#include <source_location>
#include <exception>

#include <meta>

namespace refine {

namespace detail {

// Format a value for diagnostic output using reflection
template<typename T>
consteval std::string format_value(const T& value) {
    using namespace std::meta;
    auto refl = reflect_constant(value);
    return std::string(display_string_of(refl));
}

// Build error message using reflection
template<typename T>
consteval std::string build_violation_message(const T& value) {
    return std::format(
        "Refinement violation: {} does not satisfy predicate",
        format_value(value)
    );
}

} // namespace detail

// Exception for runtime refinement failures
class refinement_error : public std::exception {
private:
    std::string message_;

public:
    template<typename T>
        requires std::formattable<T, char>
    explicit refinement_error(const T& value, std::string_view pred_name = "predicate")
        : message_(std::format("Refinement violation: {} does not satisfy {}", value, pred_name))
    {}

    template<typename T>
        requires (!std::formattable<T, char>)
    explicit refinement_error(const T&, std::string_view pred_name = "predicate")
        : message_(std::format("Refinement violation: value does not satisfy {}", pred_name))
    {}

    explicit refinement_error(std::string msg)
        : message_(std::move(msg))
    {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

// Tag type for runtime checking
struct runtime_check_t {
    explicit runtime_check_t() = default;
};
inline constexpr runtime_check_t runtime_check{};

// Tag type for unchecked construction (use with caution)
struct assume_valid_t {
    explicit assume_valid_t() = default;
};
inline constexpr assume_valid_t assume_valid{};

} // namespace refine

#endif // RCPP_DIAGNOSTICS_HPP
