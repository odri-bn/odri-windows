![logo](https://github.com/okkhor-bn/okkhor-core/blob/main/asset/okkhor.jpg)

# Okkhor

**Okkhor** is a rule-based Bangla phonetic transliteration engine.

It converts Bangla written in Latin characters into Bangla script and can also transliterate Bangla back into Latin characters.

Okkhor is designed to provide a consistent and extensible phonetic input system for applications, keyboards, and other input tools.

---

## Usage

Okkhor accepts Latin phonetic text and produces Bangla text.

For example:

```text
amar sonar bangla
```

becomes:

```text
আমার সোনার বাংলা
```

Another example:

```text
ami banglay gan gai
```

becomes:

```text
আমি বাংলায় গান গাই
```

Okkhor is phonetic rather than a character-by-character replacement system. The same Latin character can produce different Bangla characters depending on its position and surrounding characters.

---

# API

Okkhor provides a simple C++ API through the `okkhor::Engine` class.

Include the public header:

```cpp
#include <okkhor/okkhor.hpp>
```

## Creating an Engine

For normal usage, create an engine using the default constructor:

```cpp
okkhor::Engine engine;
```

The engine will use Okkhor's built-in default mappings and rules.

### Latin → Bangla

Call:

```cpp
std::string result =
    engine.transliterate_latin_to_bangla("amar sonar bangla");
```

The result will contain:

```text
আমার সোনার বাংলা
```

A complete example:

```cpp
#include <okkhor/okkhor.hpp>
#include <iostream>

int main()
{
    okkhor::Engine engine;

    std::string result =
        engine.transliterate_latin_to_bangla(
            "amar sonar bangla"
        );

    std::cout << result << '\n';
}
```

### Bangla → Latin

Use:

```cpp
std::string result =
    engine.transliterate_bangla_to_latin("আমার সোনার বাংলা");
```

A complete example:

```cpp
#include <okkhor/okkhor.hpp>
#include <iostream>

int main()
{
    okkhor::Engine engine;

    std::string result =
        engine.transliterate_bangla_to_latin(
            "আমার সোনার বাংলা"
        );

    std::cout << result << '\n';
}
```

---

# Custom Data Directory

Okkhor can optionally load its mappings and rules from a data directory instead of using the built-in defaults.

Create an engine by passing the path to your data directory:

```cpp
okkhor::Engine engine("path/to/data");
```

The directory should contain Okkhor's data files:

```text
data/
├── vowels.json
├── consonants.json
├── controls.json
├── punctuation.json
└── rules.json
```

For example:

```cpp
okkhor::Engine engine("./my-okkhor-data");

std::string result =
    engine.transliterate_latin_to_bangla(
        "amar sonar bangla"
    );
```

The custom data directory allows applications or users to **modify Okkhor's transliteration behavior** without changing the engine itself.

You can customize things such as:

* vowel mappings
* consonant mappings
* controls
* punctuation
* transliteration rules

The exact behavior of Okkhor is therefore determined by the combination of its transliteration engine and its data.

If no data directory is supplied, Okkhor uses its built-in default data.

---

# Transliteration Algorithm

Okkhor uses a **token-based, rule-driven transliteration algorithm**.

The process can be represented as:

```text
Latin Input
     │
     ▼
  Tokenization
     │
     ▼
 Rule Processing
     │
     ▼
  Orthographic
    Parsing
     │
     ▼
   Rendering
     │
     ▼
Bangla Output
```

## 1. Tokenization

The input is first divided into meaningful tokens.

Instead of immediately converting each Latin character independently, Okkhor identifies units such as:

* consonants
* vowels
* vowel signs
* controls
* punctuation
* whitespace
* other input characters

This allows Okkhor to process sequences of characters according to their context.

For example, a consonant followed by a vowel is not necessarily rendered as two independent characters. Its representation depends on where it occurs and how it interacts with the surrounding tokens.

---

## 2. Rule Processing

After tokenization, Okkhor applies its transliteration rules.

Rules determine how phonetic sequences behave according to their context.

For example, a vowel may need to become:

* an independent Bangla vowel when it begins a syllable or word
* a dependent vowel sign when it follows a consonant

Consonant sequences may also require a **হসন্ত (্)** or form conjunct consonants.

The rule system therefore considers the sequence of tokens rather than performing simple character substitution.

---

## 3. Orthographic Parsing

The processed tokens are interpreted as Bangla orthographic structures.

This stage determines how the phonetic components should be represented according to Bangla writing conventions.

For example, the same vowel sound can have different written representations depending on whether it occurs independently or after a consonant.

This stage is responsible for turning the phonetic representation into structures that correspond to actual Bangla orthography.

---

## 4. Rendering

The resulting orthographic structure is finally rendered as Unicode Bangla text.

For example:

```text
ami
```

is processed through the transliteration pipeline before becoming:

```text
আমি
```

Okkhor therefore does not simply perform:

```text
a → আ
m → ম
i → ই
```

Instead, it determines the appropriate Bangla orthographic representation from the complete sequence.

---

# Bangla → Latin

Okkhor also supports reverse transliteration.

The reverse process works conceptually as:

```text
Bangla Input
     │
     ▼
  Tokenization
     │
     ▼
 Orthographic
    Parsing
     │
     ▼
   Rendering
     │
     ▼
Latin Output
```

For example:

```text
আমার সোনার বাংলা
```

can be converted into:

```text
amar sonar bangla
```

The reverse direction uses Bangla orthographic structures to determine their corresponding Latin representation.

---

# Rule-Based Design

Okkhor's transliteration behavior is defined by **mapping data and rules** rather than being entirely hard-coded into the engine.

Conceptually:

```text
Input
  │
  ▼
Tokens
  │
  ▼
Mappings + Rules
  │
  ▼
Orthographic Structure
  │
  ▼
Output
```

This makes Okkhor configurable.

The default behavior is provided by Okkhor's built-in data, while applications can provide their own data directory when they need different mappings or rules.

---

# Phonetic Input

Okkhor is intended for users who type Bangla using Latin characters based on pronunciation.

For example:

```text
ami banglay kotha boli
```

produces:

```text
আমি বাংলায় কথা বলি
```

The goal is to allow users to type Bangla using familiar Latin characters without requiring them to memorize Unicode codes or a traditional Bangla keyboard layout.

---

# Okkhor Ecosystem

Okkhor Core is the transliteration engine used by different Okkhor input applications.

```text
                 Okkhor Core
                      │
          ┌───────────┼───────────┐
          │           │           │
          ▼           ▼           ▼
         CLI       Android      Windows
                    Keyboard     Input
```

The core focuses on transliteration, while each application provides its own user interface and platform-specific input behavior.

---

## Status

Okkhor is under active development.

The transliteration rules and orthographic behavior are continuously being refined to improve phonetic input and produce natural Bangla text.
