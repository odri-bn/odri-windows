# Okkhor Layout

This document describes the default **Okkhor Bengali phonetic input layout**.

Okkhor allows you to type Bengali using Latin characters.

For example:

```text
ami banglay likhi
```

produces:

```text
আমি বাংলায় লিখি
```

The layout is phonetic: you type Latin characters according to the sound or spelling convention used by Okkhor, and the engine converts them into Bengali Unicode.

---

# Vowels

## Independent vowels

| Input | Bengali |
| ----- | ------- |
| `o`   | অ       |
| `a`   | আ       |
| `i`   | ই       |
| `I`   | ঈ       |
| `u`   | উ       |
| `U`   | ঊ       |
| `rri` | ঋ       |
| `e`   | এ       |
| `OI`  | ঐ       |
| `O`   | ও       |
| `OU`  | ঔ       |

Examples:

```text
a  → আ
i  → ই
u  → উ
e  → এ
O  → ও
```

---

# Dependent Vowels

When a vowel follows a consonant, Bengali normally uses a dependent vowel sign.

For example:

```text
ka → কা
ki → কি
ku → কু
ke → কে
ko → কো
```

The vowel mapping therefore has both an independent and dependent form.

| Input | Independent | After consonant |
| ----- | ----------- | --------------- |
| `a`   | আ           | া               |
| `i`   | ই           | ি               |
| `I`   | ঈ           | ী               |
| `u`   | উ           | ু               |
| `U`   | ঊ           | ূ               |
| `rri` | ঋ           | ৃ               |
| `e`   | এ           | ে               |
| `OI`  | ঐ           | ৈ               |
| `O`   | ও           | ো               |
| `OU`  | ঔ           | ৌ               |

`o` represents the inherent Bengali vowel:

```text
o → অ
```

---

# Consonants

| Input | Bengali |
| ----- | ------- |
| `k`   | ক       |
| `kh`  | খ       |
| `g`   | গ       |
| `gh`  | ঘ       |
| `Ng`  | ঙ       |
| `c`   | চ       |
| `ch`  | ছ       |
| `j`   | জ       |
| `jh`  | ঝ       |
| `NG`  | ঞ       |
| `T`   | ট       |
| `Th`  | ঠ       |
| `D`   | ড       |
| `Dh`  | ঢ       |
| `N`   | ণ       |
| `t`   | ত       |
| `th`  | থ       |
| `d`   | দ       |
| `dh`  | ধ       |
| `n`   | ন       |
| `p`   | প       |
| `ph`  | ফ       |
| `b`   | ব       |
| `bh`  | ভ       |
| `v`   | ভ       |
| `m`   | ম       |
| `z`   | য       |
| `y`   | য়       |
| `r`   | র       |
| `l`   | ল       |
| `sh`  | শ       |
| `Sh`  | ষ       |
| `s`   | স       |
| `h`   | হ       |
| `R`   | ড়       |
| `Rh`  | ঢ়       |
| `t''` | ৎ       |

---

# Consonant Aliases

Some consonants can be typed in multiple ways.

## ফ

The following all produce `ফ`:

```text
ph
f
F
pH
```

## ভ

Both:

```text
bh
v
```

produce:

```text
ভ
```

## শ

Both:

```text
sh
S
```

produce:

```text
শ
```

## ষ

The following produce `ষ`:

```text
Sh
SH
```

---

# Consonant + Vowel

A consonant followed by a vowel normally produces a Bengali syllable.

Examples:

```text
ka → কা
ki → কি
ku → কু
ke → কে
ko → কো
```

More examples:

```text
ba → বা
bi → বি
bu → বু
be → বে
bo → বো
```

---

# Bengali Danda

A period represents the Bengali danda:

```text
. → ।
```

Therefore:

```text
ami bangali. 
```

can produce Bengali punctuation using:

```text
আমি বাঙালি।
```

---

# Currency

The dollar sign is mapped to the Bengali taka sign:

```text
$ → ৳
```

---

# Bengali Digits

The Latin digits are converted to Bengali digits.

| Input | Bengali |
| ----- | ------- |
| `0`   | ০       |
| `1`   | ১       |
| `2`   | ২       |
| `3`   | ৩       |
| `4`   | ৪       |
| `5`   | ৫       |
| `6`   | ৬       |
| `7`   | ৭       |
| `8`   | ৮       |
| `9`   | ৯       |

For example:

```text
2026
```

becomes:

```text
২০২৬
```

---

# Bengali Signs

## Anusvara

```text
ng → ং
```

---

## Chandrabindu

```text
^ → ঁ
```

---

## Visarga

```text
: → ঃ
```

---

## Nukta

```text
,. → ়
```

---

# Hasanta

The sequence:

```text
.,
```

produces Bengali Hasanta:

```text
্
```

Hasanta removes the inherent vowel from a consonant.

For example, a consonant followed by Hasanta can represent a dead consonant.

---

# ZWNJ

The semicolon:

```text
; 
```

represents a Zero Width Non-Joiner.

It can be used when Bengali Unicode shaping should be prevented from joining characters in the normal way.

---

# Hasanta + ZWNJ

The sequence:

```text
,,
```

is a special Okkhor shortcut.

It expands to:

```text
Hasanta + ZWNJ
```

Conceptually:

```text
,, → ., + ;
```

This is useful when a consonant needs both a Hasanta and ZWNJ.

---

# Literal Characters

Some characters have special meanings in Okkhor.

For example:

```text
.
```

normally produces:

```text
।
```

To produce a literal period:

```text
..
```

is used.

Therefore:

```text
.. → .
```

---

# Literal Semicolon

The semicolon is used as ZWNJ.

To produce a literal semicolon:

```text
;;
```

is used.

Therefore:

```text
;; → ;
```

---

# Avagraha

Okkhor uses:

```text
h'
```

for the Bengali avagraha:

```text
ঽ
```

Therefore:

```text
h' → ঽ
```

This is an **Okkhor layout convention**.

It should not be interpreted as a claim that `h'` is a universal Bengali transliteration standard.

---

# Apostrophe

A normal apostrophe can be entered directly:

```text
' → '
```

The special sequence:

```text
h'
```

is interpreted as:

```text
ঽ
```

Because `h'` is a multi-character sequence, the tokenizer must distinguish it from ordinary apostrophe usage.

---

# `w`

The letter `w` has contextual behavior.

At the beginning of a word:

```text
w → ও
```

After a consonant:

```text
w → ব
```

This allows `w` to participate in different phonetic constructions depending on its position.

---

# Special Phonetic Rules

Okkhor contains rules for sequences that require more than direct character mapping.

For example:

```text
gg
```

is transformed into:

```text
j + NG
```

and:

```text
ggg
```

is transformed into:

```text
g + g
```

These transformations allow complex Bengali forms to be represented using the normal Okkhor token system.

---

# Compound Input

Okkhor supports multi-character phonetic units.

Examples:

```text
kh
gh
ch
jh
th
dh
bh
sh
Sh
Rh
rri
```

These are treated as phonetic tokens rather than independent characters.

Therefore:

```text
kh
```

means:

```text
খ
```

rather than:

```text
ক + হ
```

---

# Ambiguous Sequences

Some special sequences overlap with normal phonetic tokens.

For example:

```text
bh'
```

contains:

```text
bh
```

and:

```text
h'
```

The intended interpretation is:

```text
bh + '
```

rather than:

```text
b + h'
```

Similarly, sequences involving:

```text
kh
gh
dh
```

must preserve those consonant tokens.

Okkhor's tokenizer therefore applies token precedence rules when handling overlapping sequences.

---

# Escaping

Several ASCII characters have special meanings in the layout.

| Character | Meaning                   |
| --------- | ------------------------- |
| `.`       | Bengali danda             |
| `,`       | Part of control sequences |
| `;`       | ZWNJ                      |
| `'`       | Literal apostrophe        |
| `` ` ``   | Virtual control           |
| `:`       | Visarga                   |
| `^`       | Chandrabindu              |
| `$`       | Taka                      |

Escape sequences allow these characters to be entered literally when necessary.

Current examples:

```text
.. → .
;; → ;
```

and:

```text
,, → Hasanta + ZWNJ
```

---

# Quick Reference

## Vowels

```text
o  → অ
a  → আ
i  → ই
I  → ঈ
u  → উ
U  → ঊ
rri → ঋ
e  → এ
OI → ঐ
O  → ও
OU → ঔ
```

## Common consonants

```text
k  → ক
kh → খ
g  → গ
gh → ঘ
c  → চ
ch → ছ
j  → জ
jh → ঝ
T  → ট
Th → ঠ
D  → ড
Dh → ঢ
t  → ত
th → থ
d  → দ
dh → ধ
n  → ন
p  → প
ph → ফ
b  → ব
bh → ভ
m  → ম
r  → র
l  → ল
sh → শ
Sh → ষ
s  → স
h  → হ
```

## Controls and special input

```text
., → ্
;  → ZWNJ
,, → Hasanta + ZWNJ

.  → ।
.. → .

;; → ;

ng → ং
^  → ঁ
:  → ঃ
,. → ়

$  → ৳
h' → ঽ
'  → '
```

## Digits

```text
0 → ০
1 → ১
2 → ২
3 → ৩
4 → ৪
5 → ৫
6 → ৬
7 → ৭
8 → ৮
9 → ৯
```

---

# Examples

### আমি

```text
ami
```

→

```text
আমি
```

### বাংলা

```text
bangla
```

→

```text
বাংলা
```

### বাংলাদেশ

```text
bangladesh
```

→

```text
বাংলাদেশ
```

### লিখি

```text
likhi
```

→

```text
লিখি
```

### টাকা

```text
$100
```

→

```text
৳১০০
```

### Avagraha

```text
h'
```

→

```text
ঽ
```

---

# Layout Philosophy

The Okkhor layout is designed around three ideas:

### Phonetic input

Users type Latin characters corresponding to Bengali sounds.

### Compact controls

Frequently needed Bengali orthographic controls have short ASCII representations.

### Context-aware typing

Some sequences are interpreted according to their surrounding characters rather than by fixed one-to-one substitution.

This allows the layout to remain relatively compact while supporting more complex Bengali orthography.
