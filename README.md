# hclx

![CI](https://github.com/richardthorell/hclx/actions/workflows/ci.yml/badge.svg)

hclx is a modern C++20 library for parsing a focused, well defined subset of HashiCorp Configuration Language (HCL). It is designed for embedding into systems software, tooling, game engines, build systems, and other performance sensitive environments.

The library emphasizes:

* Clear and minimal public API
* Strong diagnostics without exceptions
* Value based AST representation
* Header only public interface with a compiled static library backend
* No external runtime dependencies

hclx is suitable when you need a lightweight, embeddable HCL style configuration parser without pulling in large external frameworks.

---

## Status

This project is under active development. The currently supported grammar focuses on:

* Blocks
* Attributes
* Primitive literals (string, integer, double, bool, null)
* Arrays
* Objects
* Configurable comment styles

The goal is correctness, clarity, and performance for a constrained and well documented subset rather than full Terraform compatibility.

---

## Installation

### Using CMake

```cmake
add_subdirectory(hclx)

target_link_libraries(your_target PRIVATE hclx)
```

Minimum requirements:

* C++20 compatible compiler
* CMake 3.20 or newer

The library builds as a static library by default.

---

## Quick Example

```cpp
#include <hclx/parser.h>
#include <string>

int main()
{
    std::string input = R"(
        server "api" {
            port = 8080
            enabled = true
        }
    )";

    auto result = hclx::parse(
        std::as_bytes(std::span(input))
    );

    if (!result.ast)
    {
        for (const auto& d : result.diags.items)
        {
            // handle diagnostics
        }
        return 1;
    }

    const hclx::ast& tree = *result.ast;

    for (const auto& item : tree.items)
    {
        if (std::holds_alternative<hclx::block>(item))
        {
            const auto& blk = std::get<hclx::block>(item);
            // inspect block
        }
    }
}
```

---

## API Overview

### Entry Points

```cpp
hclx::parse(std::span<const std::byte> input,
            hclx::parse_options options = {});

hclx::parse_file(std::string_view filename,
                 hclx::parse_options options = {});
```

### Result Type

```cpp
struct parse_result
{
    std::optional<ast> ast;
    diagnostics diags;
};
```

Parsing never throws for syntax errors. All issues are reported through structured diagnostics.

---

## Diagnostics

Diagnostics contain:

* Severity
* Source range
* Message

The caller retains full control over formatting, logging, and error handling strategy.

---

## Building Tests

```bash
cmake -S . -B build -DHCLX_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```

Tests use GoogleTest via FetchContent.

---

## License

MIT License.
