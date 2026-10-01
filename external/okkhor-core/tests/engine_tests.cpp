#include <okkhor/okkhor.hpp>
#include <string>

#include "test_util.hpp"

using namespace testing;

#define TEST_ROUND_TRIP(text)                                                  \
  check("Round trip: " #text,                                                  \
        engine.transliterate_latin_to_bangla(                                  \
            engine.transliterate_bangla_to_latin(text)),                       \
        text)

int main() {
  const okkhor::Engine engine;

  // Special case
  check("Latin -> Bangla: o", engine.transliterate_latin_to_bangla("kowa"),
        "কয়া");
  check("Latin -> Bangla: o", engine.transliterate_latin_to_bangla("``"), "`");
  check("Latin -> Bangla: o", engine.transliterate_latin_to_bangla("paOwa"),"পাওয়া");
  check("Latin -> Bangla: o", engine.transliterate_latin_to_bangla("haOwa"),"হাওয়া");

  // -------------------------------------------------------------------------
  // Basic Latin -> Bangla vowels
  // -------------------------------------------------------------------------

  check("Latin -> Bangla: o", engine.transliterate_latin_to_bangla("o"), "অ");

  check("Latin -> Bangla: a", engine.transliterate_latin_to_bangla("a"), "আ");

  check("Latin -> Bangla: i", engine.transliterate_latin_to_bangla("i"), "ই");

  check("Latin -> Bangla: I", engine.transliterate_latin_to_bangla("I"), "ঈ");

  check("Latin -> Bangla: u", engine.transliterate_latin_to_bangla("u"), "উ");

  check("Latin -> Bangla: U", engine.transliterate_latin_to_bangla("U"), "ঊ");

  check("Latin -> Bangla: rri", engine.transliterate_latin_to_bangla("rri"),
        "ঋ");

  check("Latin -> Bangla: e", engine.transliterate_latin_to_bangla("e"), "এ");

  check("Latin -> Bangla: OI", engine.transliterate_latin_to_bangla("OI"), "ঐ");

  check("Latin -> Bangla: O", engine.transliterate_latin_to_bangla("O"), "ও");

  check("Latin -> Bangla: OU", engine.transliterate_latin_to_bangla("OU"), "ঔ");

  // -------------------------------------------------------------------------
  // Basic Latin -> Bangla consonants
  // -------------------------------------------------------------------------

  check("Latin -> Bangla: k", engine.transliterate_latin_to_bangla("k"), "ক");

  check("Latin -> Bangla: kh", engine.transliterate_latin_to_bangla("kh"), "খ");

  check("Latin -> Bangla: g", engine.transliterate_latin_to_bangla("g"), "গ");

  check("Latin -> Bangla: gh", engine.transliterate_latin_to_bangla("gh"), "ঘ");

  check("Latin -> Bangla: Ng", engine.transliterate_latin_to_bangla("Ng"), "ঙ");

  check("Latin -> Bangla: c", engine.transliterate_latin_to_bangla("c"), "চ");

  check("Latin -> Bangla: ch", engine.transliterate_latin_to_bangla("ch"), "ছ");

  check("Latin -> Bangla: j", engine.transliterate_latin_to_bangla("j"), "জ");

  check("Latin -> Bangla: jh", engine.transliterate_latin_to_bangla("jh"), "ঝ");

  check("Latin -> Bangla: NG", engine.transliterate_latin_to_bangla("NG"), "ঞ");

  check("Latin -> Bangla: T", engine.transliterate_latin_to_bangla("T"), "ট");

  check("Latin -> Bangla: Th", engine.transliterate_latin_to_bangla("Th"), "ঠ");

  check("Latin -> Bangla: D", engine.transliterate_latin_to_bangla("D"), "ড");

  check("Latin -> Bangla: Dh", engine.transliterate_latin_to_bangla("Dh"), "ঢ");

  check("Latin -> Bangla: N", engine.transliterate_latin_to_bangla("N"), "ণ");

  check("Latin -> Bangla: t", engine.transliterate_latin_to_bangla("t"), "ত");

  check("Latin -> Bangla: th", engine.transliterate_latin_to_bangla("th"), "থ");

  check("Latin -> Bangla: d", engine.transliterate_latin_to_bangla("d"), "দ");

  check("Latin -> Bangla: dh", engine.transliterate_latin_to_bangla("dh"), "ধ");

  check("Latin -> Bangla: n", engine.transliterate_latin_to_bangla("n"), "ন");

  check("Latin -> Bangla: p", engine.transliterate_latin_to_bangla("p"), "প");

  check("Latin -> Bangla: ph", engine.transliterate_latin_to_bangla("ph"), "ফ");

  check("Latin -> Bangla: b", engine.transliterate_latin_to_bangla("b"), "ব");

  check("Latin -> Bangla: bh", engine.transliterate_latin_to_bangla("bh"), "ভ");

  check("Latin -> Bangla: v", engine.transliterate_latin_to_bangla("v"), "ভ");

  check("Latin -> Bangla: m", engine.transliterate_latin_to_bangla("m"), "ম");

  check("Latin -> Bangla: z", engine.transliterate_latin_to_bangla("z"), "য");

  check("Latin -> Bangla: y", engine.transliterate_latin_to_bangla("y"), "য়");

  check("Latin -> Bangla: r", engine.transliterate_latin_to_bangla("r"), "র");

  check("Latin -> Bangla: l", engine.transliterate_latin_to_bangla("l"), "ল");

  check("Latin -> Bangla: sh", engine.transliterate_latin_to_bangla("sh"), "শ");

  check("Latin -> Bangla: Sh", engine.transliterate_latin_to_bangla("Sh"), "ষ");

  check("Latin -> Bangla: s", engine.transliterate_latin_to_bangla("s"), "স");

  check("Latin -> Bangla: h", engine.transliterate_latin_to_bangla("h"), "হ");

  check("Latin -> Bangla: R", engine.transliterate_latin_to_bangla("R"), "ড়");

  check("Latin -> Bangla: Rh", engine.transliterate_latin_to_bangla("Rh"), "ঢ়");

  check("Latin -> Bangla: t''", engine.transliterate_latin_to_bangla("t''"),
        "ৎ");

  // -------------------------------------------------------------------------
  // Consonant aliases
  // -------------------------------------------------------------------------

  check("Alias: f -> ph", engine.transliterate_latin_to_bangla("f"), "ফ");

  check("Alias: F -> ph", engine.transliterate_latin_to_bangla("F"), "ফ");

  check("Alias: pH -> ph", engine.transliterate_latin_to_bangla("pH"), "ফ");

  check("Alias: S -> sh", engine.transliterate_latin_to_bangla("S"), "শ");

  check("Alias: SH -> Sh", engine.transliterate_latin_to_bangla("SH"), "ষ");

  check("Alias: v -> bh", engine.transliterate_latin_to_bangla("v"), "ভ");

  // -------------------------------------------------------------------------
  // Basic vowel attachment
  // -------------------------------------------------------------------------

  check("Latin -> Bangla: ka", engine.transliterate_latin_to_bangla("ka"),
        "কা");

  check("Latin -> Bangla: ki", engine.transliterate_latin_to_bangla("ki"),
        "কি");

  check("Latin -> Bangla: kI", engine.transliterate_latin_to_bangla("kI"),
        "কী");

  check("Latin -> Bangla: ku", engine.transliterate_latin_to_bangla("ku"), "কু");

  check("Latin -> Bangla: kU", engine.transliterate_latin_to_bangla("kU"), "কূ");

  check("Latin -> Bangla: krri", engine.transliterate_latin_to_bangla("krri"),
        "কৃ");

  check("Latin -> Bangla: ke", engine.transliterate_latin_to_bangla("ke"),
        "কে");

  check("Latin -> Bangla: kOI", engine.transliterate_latin_to_bangla("kOI"),
        "কৈ");

  check("Latin -> Bangla: kO", engine.transliterate_latin_to_bangla("kO"),
        "কো");

  check("Latin -> Bangla: kOU", engine.transliterate_latin_to_bangla("kOU"),
        "কৌ");

  // -------------------------------------------------------------------------
  // Structural controls
  // -------------------------------------------------------------------------

  check("Latin -> Bangla: explicit hasanta",
        engine.transliterate_latin_to_bangla("k.,"), "ক্");

  check("Latin -> Bangla: hasanta + ZWNJ macro",
        engine.transliterate_latin_to_bangla("k,,"), "ক্‌");

  check("Latin -> Bangla: ZWNJ", engine.transliterate_latin_to_bangla("k;"),
        "ক‌");

  check("Latin -> Bangla: virtual consonant",
        engine.transliterate_latin_to_bangla("k`"), "ক");

  // -------------------------------------------------------------------------
  // Accents / nukta
  // -------------------------------------------------------------------------

  check("Latin -> Bangla: anusvara",
        engine.transliterate_latin_to_bangla("kng"), "কং");

  check("Latin -> Bangla: chandrabindu",
        engine.transliterate_latin_to_bangla("k^"), "কঁ");

  check("Latin -> Bangla: visarga", engine.transliterate_latin_to_bangla("k:"),
        "কঃ");

  check("Latin -> Bangla: nukta", engine.transliterate_latin_to_bangla("k,."),
        "ক়");

  // -------------------------------------------------------------------------
  // Rules: kkh
  // -------------------------------------------------------------------------

  check("Rule: kkh", engine.transliterate_latin_to_bangla("kkh"), "ক্ষ");

  check("Rule: kkh + vowel", engine.transliterate_latin_to_bangla("kkha"),
        "ক্ষা");

  // -------------------------------------------------------------------------
  // Rules: gg
  // -------------------------------------------------------------------------

  check("Rule: gg", engine.transliterate_latin_to_bangla("gg"), "জ্ঞ");

  check("Rule: gg + vowel", engine.transliterate_latin_to_bangla("gga"), "জ্ঞা");

  // -------------------------------------------------------------------------
  // Rules: ggg
  // -------------------------------------------------------------------------

  check("Rule: ggg", engine.transliterate_latin_to_bangla("ggg"), "গ্গ");

  check("Rule: ggg + vowel", engine.transliterate_latin_to_bangla("ggga"),
        "গ্গা");

  // -------------------------------------------------------------------------
  // Rules: ng
  // -------------------------------------------------------------------------

  check("Rule: ng before vowel", engine.transliterate_latin_to_bangla("nga"),
        "ঙা");

  check("Rule: ng before consonant",
        engine.transliterate_latin_to_bangla("ngk"), "ঙ‌ক");

  // -------------------------------------------------------------------------
  // Rules: w
  // -------------------------------------------------------------------------

  check("Rule: w at word start", engine.transliterate_latin_to_bangla("w"),
        "ও");

  check("Rule: w after consonant", engine.transliterate_latin_to_bangla("kw"),
        "ক্ব");

  // -------------------------------------------------------------------------
  // Rules: ,,
  // -------------------------------------------------------------------------

  check("Rule: ,,", engine.transliterate_latin_to_bangla(",,"), "্‌");

  check("Rule: k,,", engine.transliterate_latin_to_bangla("k,,"), "ক্‌");

  // -------------------------------------------------------------------------
  // Rules: ;,
  // -------------------------------------------------------------------------

  check("Rule: ;,", engine.transliterate_latin_to_bangla(";,"), "‌,");

  check("Rule: k;,", engine.transliterate_latin_to_bangla("k;,"), "ক‌,");

  // -------------------------------------------------------------------------
  // Rules: rya
  // -------------------------------------------------------------------------

  check("Rule: rya", engine.transliterate_latin_to_bangla("rya"),
        "র‍্যা");

  // -------------------------------------------------------------------------
  // Rules: ..
  // -------------------------------------------------------------------------

  check("Rule: ..", engine.transliterate_latin_to_bangla(".."), ".");

  check("Rule: .. after word", engine.transliterate_latin_to_bangla("ka.."),
        "কা.");

  // -------------------------------------------------------------------------
  // Bengali digits
  // -------------------------------------------------------------------------

  check("Digit: 0", engine.transliterate_latin_to_bangla("0"), "০");

  check("Digit: 1", engine.transliterate_latin_to_bangla("1"), "১");

  check("Digit: 2", engine.transliterate_latin_to_bangla("2"), "২");

  check("Digit: 3", engine.transliterate_latin_to_bangla("3"), "৩");

  check("Digit: 4", engine.transliterate_latin_to_bangla("4"), "৪");

  check("Digit: 5", engine.transliterate_latin_to_bangla("5"), "৫");

  check("Digit: 6", engine.transliterate_latin_to_bangla("6"), "৬");

  check("Digit: 7", engine.transliterate_latin_to_bangla("7"), "৭");

  check("Digit: 8", engine.transliterate_latin_to_bangla("8"), "৮");

  check("Digit: 9", engine.transliterate_latin_to_bangla("9"), "৯");

  // -------------------------------------------------------------------------
  // Bangla -> Latin: structural controls
  // -------------------------------------------------------------------------

  check("Bangla -> Latin: explicit hasanta",
        engine.transliterate_bangla_to_latin("ক্"), "k.,");

  check("Bangla -> Latin: hasanta + ZWNJ",
        engine.transliterate_bangla_to_latin("ক্‌"), "k,,");

  check("Bangla -> Latin: ZWNJ", engine.transliterate_bangla_to_latin("ক‌"),
        "k;");

  // -------------------------------------------------------------------------
  // Bangla -> Latin: basic vowels
  // -------------------------------------------------------------------------

  check("Bangla -> Latin: independent অ",
        engine.transliterate_bangla_to_latin("অ"), "o");

  check("Bangla -> Latin: independent আ",
        engine.transliterate_bangla_to_latin("আ"), "a");

  check("Bangla -> Latin: independent ই",
        engine.transliterate_bangla_to_latin("ই"), "i");

  check("Bangla -> Latin: independent ঈ",
        engine.transliterate_bangla_to_latin("ঈ"), "I");

  check("Bangla -> Latin: independent উ",
        engine.transliterate_bangla_to_latin("উ"), "u");

  check("Bangla -> Latin: independent ঊ",
        engine.transliterate_bangla_to_latin("ঊ"), "U");

  check("Bangla -> Latin: independent ঋ",
        engine.transliterate_bangla_to_latin("ঋ"), "rri");

  check("Bangla -> Latin: independent এ",
        engine.transliterate_bangla_to_latin("এ"), "e");

  check("Bangla -> Latin: independent ঐ",
        engine.transliterate_bangla_to_latin("ঐ"), "OI");

  check("Bangla -> Latin: independent ও",
        engine.transliterate_bangla_to_latin("ও"), "O");

  check("Bangla -> Latin: independent ঔ",
        engine.transliterate_bangla_to_latin("ঔ"), "OU");

  // -------------------------------------------------------------------------
  // Bangla -> Latin: dependent vowels
  // -------------------------------------------------------------------------

  check("Bangla -> Latin: কা", engine.transliterate_bangla_to_latin("কা"),
        "ka");

  check("Bangla -> Latin: কি", engine.transliterate_bangla_to_latin("কি"),
        "ki");

  check("Bangla -> Latin: কী", engine.transliterate_bangla_to_latin("কী"),
        "kI");

  check("Bangla -> Latin: কু", engine.transliterate_bangla_to_latin("কু"), "ku");

  check("Bangla -> Latin: কূ", engine.transliterate_bangla_to_latin("কূ"), "kU");

  check("Bangla -> Latin: কৃ", engine.transliterate_bangla_to_latin("কৃ"),
        "krri");

  check("Bangla -> Latin: কে", engine.transliterate_bangla_to_latin("কে"),
        "ke");

  check("Bangla -> Latin: কৈ", engine.transliterate_bangla_to_latin("কৈ"),
        "kOI");

  check("Bangla -> Latin: কো", engine.transliterate_bangla_to_latin("কো"),
        "kO");

  check("Bangla -> Latin: কৌ", engine.transliterate_bangla_to_latin("কৌ"),
        "kOU");

  // -------------------------------------------------------------------------
  // Bangla -> Latin: accents
  // -------------------------------------------------------------------------

  check("Bangla -> Latin: anusvara", engine.transliterate_bangla_to_latin("কং"),
        "kng");

  check("Bangla -> Latin: chandrabindu",
        engine.transliterate_bangla_to_latin("কঁ"), "k^");

  check("Bangla -> Latin: nukta", engine.transliterate_bangla_to_latin("ক়"),
        "k,.");

  // -------------------------------------------------------------------------
  // Round trip: simple vowels
  // -------------------------------------------------------------------------

  check("Round trip: a",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("a")),
        "a");

  check("Round trip: i",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("i")),
        "i");

  check("Round trip: I",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("I")),
        "I");

  check("Round trip: u",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("u")),
        "u");

  check("Round trip: U",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("U")),
        "U");

  check("Round trip: rri",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("rri")),
        "rri");

  check("Round trip: e",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("e")),
        "e");

  check("Round trip: OI",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("OI")),
        "OI");

  check("Round trip: O",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("O")),
        "O");

  check("Round trip: OU",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("OU")),
        "OU");

  // -------------------------------------------------------------------------
  // Round trip: structural controls
  // -------------------------------------------------------------------------

  check("Round trip: hasanta",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্")),
        "ক্");

  check("Round trip: hasanta + ZWNJ",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্‌")),
        "ক্‌");

  check("Round trip: consonant + hasanta + consonant",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্‌র")),
        "ক্‌র");

  check("Round trip: multiple ZWNJ-separated consonants",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্‌ন্‌ম")),
        "ক্‌ন্‌ম");

  // -------------------------------------------------------------------------
  // Round trip: conjuncts
  // -------------------------------------------------------------------------

  check("Round trip: simple conjunct",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্ন")),
        "ক্ন");

  check("Round trip: conjunct + vowel",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্না")),
        "ক্না");

  check("Round trip: three-consonant conjunct",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্ন্ত")),
        "ক্ন্ত");

  check("Round trip: ক্ষ",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্ষ")),
        "ক্ষ");

  check("Round trip: ক্ষ + vowel",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্ষি")),
        "ক্ষি");

  check("Round trip: conjunct + macro",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্ষ্‌র")),
        "ক্ষ্‌র");

  // -------------------------------------------------------------------------
  // Round trip: accents
  // -------------------------------------------------------------------------

  check("Round trip: chandrabindu",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("কাঁ")),
        "কাঁ");

  check("Round trip: anusvara",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("বং")),
        "বং");

  check("Round trip: visarga",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("কঃ")),
        "কঃ");

  check("Round trip: nukta",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক়")),
        "ক়");

  check("Round trip: conjunct + chandrabindu",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("ক্ষাঁ")),
        "ক্ষাঁ");

  //
  // -------------------------------------------------------------------------
  // Round trip: rules
  //
  // -------------------------------------------------------------------------

  check("Round trip: kkh",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("kkh")),
        "kkh");

  check("Round trip: gg",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("gg")),
        "gg");

  check("Round trip: ggg",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("ggg")),
        "ggg");

  check("Round trip: ,,",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla(",,")),
        ",,");

  check("Round trip: ;,",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla(";,")),
        ";,");

  // -------------------------------------------------------------------------
  // Round trip: common words
  // -------------------------------------------------------------------------

  check("Round trip: amar",
        engine.transliterate_bangla_to_latin(
            engine.transliterate_latin_to_bangla("amar")),
        "amar");

  check("Round trip: বাংলা",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("বাংলা")),
        "বাংলা");

  check("Round trip: মন্ত্র",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("মন্ত্র")),
        "মন্ত্র");

  // -------------------------------------------------------------------------
  // Longer real-world words
  // -------------------------------------------------------------------------

  const std::string bangla_words[] = {
      "বাংলাদেশ",  "রাজশাহী",   "পদার্থবিজ্ঞান", "কম্পিউটার",   "বিজ্ঞান",
      "প্রযুক্তি",    "সফটওয়্যার",  "হার্ডওয়্যার",   "গণিত",       "বিশ্ববিদ্যালয়",
      "প্রোগ্রামিং", "অ্যালগরিদম", "সিমুলেশন",     "অপ্টিমাইজেশন"};

  for (const std::string &word : bangla_words) {
    check("Round trip: " + word,
          engine.transliterate_latin_to_bangla(
              engine.transliterate_bangla_to_latin(word)),
          word);
  }

  // -------------------------------------------------------------------------
  // Sentences / whitespace / punctuation
  // -------------------------------------------------------------------------

  check("Round trip: whitespace",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("বাংলা ভাষা")),
        "বাংলা ভাষা");

  check("Round trip: multiple spaces",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("বাংলা  ভাষা")),
        "বাংলা  ভাষা");

  check("Round trip: punctuation",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("বাংলা।")),
        "বাংলা।");

  check("Round trip: currency",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("৳")),
        "৳");

  check("Round trip: Bengali digits",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("১২৩৪৫৬৭৮৯০")),
        "১২৩৪৫৬৭৮৯০");

  // -------------------------------------------------------------------------
  // Mixed content
  // -------------------------------------------------------------------------

  check(
      "Round trip: Bangla sentence",
      engine.transliterate_latin_to_bangla(engine.transliterate_bangla_to_latin(
          "আমি বাংলা ভাষায় প্রোগ্রামিং করি।")),
      "আমি বাংলা ভাষায় প্রোগ্রামিং করি।");

  check("Round trip: Bangla + digits",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin("বাংলাদেশ ২০২৬")),
        "বাংলাদেশ ২০২৬");

  // -------------------------------------------------------------------------
  // Original hard cases
  // -------------------------------------------------------------------------

  const std::string hard = "ক্ষ্ম্যজ্ঞ্যন্ত্‌র্য্যশ্চ্ছ্বঙ্ক্ষ্ট্র্দ্ধ্ব্যাঁক্‌ন্‌ম্গ্ন্ত্র্"
                           "য"
                           "ৌ"
                           "ক"
                           "্"
                           "ক"
                           "্"
                           "হ"
                           "ৃ"
                           "ঙ"
                           "্"
                           "ঘ"
                           "্"
                           "ন"
                           "্"
                           "য";

  const std::string soft = "ক্ক্হৃ";

  check("Round trip: hard",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin(hard)),
        hard);

  check("Round trip: soft",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin(soft)),
        soft);

  check("Round trip: hard + soft",
        engine.transliterate_latin_to_bangla(
            engine.transliterate_bangla_to_latin(hard + soft)),
        hard + soft);

  TEST_ROUND_TRIP("नमस्ते");
  TEST_ROUND_TRIP("हिन्दी");
  TEST_ROUND_TRIP("भारत");
  TEST_ROUND_TRIP("வணக்கம்");
  TEST_ROUND_TRIP("தமிழ்");

  // -------------------------------------------------------------------------
  // Final report
  // -------------------------------------------------------------------------

  return testing::report("engine");
}