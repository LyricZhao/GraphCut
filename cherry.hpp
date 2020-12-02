#pragma once

#include <cctype>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <ostream>
#include <random>
#include <set>
#include <string>
#include <type_traits>


// Returning a const value is not recommended by Clang-Tidy for readability, but for functionality we need it
#pragma ide diagnostic ignored "readability-const-return-type"
// There are some macros maybe unused
#pragma ide diagnostic ignored "OCUnusedMacroInspection"


/// A nanosecond-level timer
class [[maybe_unused]] NanoTimer {
private:
    typedef std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> time_point_t;

    time_point_t last_time_point;

public:
    [[maybe_unused]] NanoTimer() {
        last_time_point = std::chrono::system_clock::now();
    }

    /// Return the duration from last time point (constructor `Timer()` or `tik()`)
    [[maybe_unused]] uint64_t tik() {
        time_point_t time_point = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(time_point - last_time_point);
        last_time_point = time_point;
        return duration.count();
    }
};


/// A random number generator
template <typename value_type>
class [[maybe_unused]] Random {
private:
    typedef typename std::conditional<std::is_integral<value_type>::value,
            std::uniform_int_distribution<value_type>, std::uniform_real_distribution<value_type>>::type dist_t;

    std::default_random_engine engine;
    dist_t dist;

public:
    /// The interval is closed ([`min`, `max`])
    [[maybe_unused]] Random(value_type min, value_type max, int seed=0, bool pure=true) {
        assert(min <= max);
        if (pure) {
            seed = std::random_device()();
        }
        engine = std::default_random_engine(seed);
        dist = dist_t(min, max);
    }

    /// Generate a random number
    [[maybe_unused]] value_type operator ()() {
        return dist(engine);
    }
};


// TODO: Support enumerate with index
// NOTE: The compiler error may not be friendly when use `iterator` as `const_iterator` by force


/// A shifted range wrapper
template <typename Range>
class [[maybe_unused]] ShiftRange {
private:
    int pos, length = -1;
    Range &range;

public:
    typedef typename std::conditional<std::is_const<Range>::value,
            typename Range::const_iterator, typename Range::iterator>::type iterator;
    typedef typename std::conditional<std::is_const<Range>::value,
            typename Range::const_reverse_iterator, typename Range::reverse_iterator>::type reverse_iterator;
    typedef typename Range::const_iterator const_iterator;
    typedef typename Range::const_reverse_iterator const_reverse_iterator;
    typedef typename Range::value_type value_type;

    explicit ShiftRange(Range &range, int pos=0, int length=-1): range(range), pos(pos) {
        this->length = length == -1 ? range.end() - range.begin() - pos : length;
        assert(pos + this->length <= range.end() - range.begin());
    }

    [[maybe_unused]] [[nodiscard]] iterator begin() {
        return range.begin() + pos;
    }

    [[maybe_unused]] [[nodiscard]] iterator end() {
        return range.begin() + pos + length;
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rbegin() {
        auto cut = range.end() - range.begin() - (pos + length);
        return range.rbegin() + cut;
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rend() {
        auto cut = range.end() - range.begin() - pos;
        return range.rbegin() + cut;
    }

    [[maybe_unused]] [[nodiscard]] const_iterator begin() const {
        return range.begin() + pos;
    }

    [[maybe_unused]] [[nodiscard]] const_iterator end() const {
        return range.begin() + pos + length;
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rbegin() const {
        auto cut = range.end() - range.begin() - (pos + length);
        return range.rbegin() + cut;
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rend() const {
        auto cut = range.end() - range.begin() - pos;
        return range.rbegin() + cut;
    }
};


/// Return a shifted range wrapper (uses left-value reference)
template <typename Range>
[[maybe_unused]] [[nodiscard]] ShiftRange<Range> shift(Range &range, int pos=0, int length=-1) {
    return ShiftRange(range, pos, length);
}


/// Return a shifted range wrapper (uses const reference)
template <typename Range>
[[maybe_unused]] [[nodiscard]] const ShiftRange<const Range> shift(const Range &range, int pos=0, int length=-1) {
    return ShiftRange<const Range>(range, pos, length);
}


/// A reversed range wrapper
template <typename Range>
class [[maybe_unused]] ReversedRange {
private:
    Range &range;

public:
    typedef typename std::conditional<std::is_const<Range>::value, typename Range::const_reverse_iterator, typename Range::reverse_iterator>::type iterator;
    typedef typename std::conditional<std::is_const<Range>::value, typename Range::const_iterator, typename Range::iterator>::type reverse_iterator;
    typedef typename Range::const_reverse_iterator const_iterator;
    typedef typename Range::const_iterator const_reverse_iterator;
    typedef typename Range::value_type value_type;

    explicit ReversedRange(Range &range): range(range) {}

    [[maybe_unused]] [[nodiscard]] iterator begin() {
        return range.rbegin();
    }

    [[maybe_unused]] [[nodiscard]] iterator end() {
        return range.rend();
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rbegin() {
        return range.begin();
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rend() {
        return range.end();
    }

    [[maybe_unused]] [[nodiscard]] const_iterator begin() const {
        return range.rbegin();
    }

    [[maybe_unused]] [[nodiscard]] const_iterator end() const {
        return range.rend();
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rbegin() const {
        return range.begin();
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rend() const {
        return range.end();
    }
};


/// Return a reversed range wrapper (uses left-value reference)
template <typename Range>
[[maybe_unused]] [[nodiscard]] ReversedRange<Range> reverse(Range &range) {
    return ReversedRange<Range>(range);
}


/// Return a reversed range wrapper (uses right-value)
template <typename Range>
[[maybe_unused]] [[nodiscard]] ReversedRange<Range> reverse(Range &&range) {
    return ReversedRange<Range>(range);
}


/// Return a reversed range wrapper (uses const reference)
template <typename Range>
[[maybe_unused]] [[nodiscard]] const ReversedRange<const Range> reverse(const Range &range) {
    return ReversedRange<const Range>(range);
}


/// A range iterated by an indexing array
template <typename Array, typename Range>
class [[maybe_unused]] IndexingRange {
private:
    Array &items;
    const Range &indexes;

public:
    typedef typename Array::value_type value_type;
    typedef typename std::conditional<std::is_const<Array>::value,
            const typename Array::value_type&, typename Array::value_type&>::type reference;

    /// The iterator type for `IndexingRange`
    template<typename index_const_iterator_t>
    struct [[maybe_unused]] Iterator {
        Array &items;
        index_const_iterator_t index_const_iterator;

        [[maybe_unused]] Iterator(Array &items, const index_const_iterator_t &index_const_iterator):
                items(items), index_const_iterator(index_const_iterator) {}

        [[maybe_unused]] reference operator *() const {
            return items[*index_const_iterator];
        }

        [[maybe_unused]] Iterator operator ++() {
            index_const_iterator ++;
            return *this;
        }

        [[maybe_unused]] bool operator ==(const Iterator &other) const {
            return index_const_iterator == other.index_const_iterator;
        }

        [[maybe_unused]] bool operator !=(const Iterator &other) const {
            return index_const_iterator != other.index_const_iterator;
        }
    };

    typedef Iterator<typename Range::const_iterator> iterator;
    typedef Iterator<typename Range::const_reverse_iterator> reverse_iterator;
    typedef Iterator<typename Range::const_iterator> const_iterator;
    typedef Iterator<typename Range::const_reverse_iterator> const_reverse_iterator;

    [[maybe_unused]] IndexingRange(Array &items, const Range &indexes):
            items(items), indexes(indexes) {}

    [[maybe_unused]] [[nodiscard]] iterator begin() {
        return iterator(items, indexes.begin());
    }

    [[maybe_unused]] [[nodiscard]] iterator end() {
        return iterator(items, indexes.end());
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rbegin() {
        return reverse_iterator(items, indexes.rbegin());
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rend() {
        return reverse_iterator(items, indexes.rend());
    }

    [[maybe_unused]] [[nodiscard]] const_iterator begin() const {
        return const_iterator(items, indexes.begin());
    }

    [[maybe_unused]] [[nodiscard]] const_iterator end() const {
        return const_iterator(items, indexes.end());
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rbegin() const {
        return const_reverse_iterator(items, indexes.rbegin());
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rend() const {
        return const_reverse_iterator(items, indexes.rend());
    }
};


/// Return a range iterated by an indexing array (`array` is left-value reference)
template <typename Array, typename Range>
[[maybe_unused]] [[nodiscard]] IndexingRange<Array, Range> indexing(Array &array, const Range &indexes) {
    return IndexingRange<Array, Range>(array, indexes);
}


/// Return a range iterated by an indexing array (`array` is const reference)
template <typename Array, typename Range>
[[maybe_unused]] [[nodiscard]] const IndexingRange<const Array, Range> indexing(const Array &array, const Range &indexes) {
    return IndexingRange<const Array, Range>(array, indexes);
}


/// A joined range for two ranges
template <typename Range1, typename Range2>
class [[maybe_unused]] JoinedRange {
private:
    Range1 &range1;
    Range2 &range2;

public:
    // Check whether they're the same type
    static_assert(std::is_same<typename Range1::value_type, typename Range2::value_type>::value,
                  "The types of two ranges in JoinedRange must be same");

    static constexpr bool be_const = std::is_const<Range1>::value or std::is_const<Range2>::value;
    typedef typename Range1::value_type value_type;
    typedef typename std::conditional<be_const,
            const typename Range1::value_type&, typename Range1::value_type&>::type reference;

    /// The iterator type for `JoinedRange`
    template <typename iterator1_t, typename iterator2_t>
    struct [[maybe_unused]] Iterator {
        bool first;
        iterator1_t iterator1, iterator1_end;
        iterator2_t iterator2;

        [[maybe_unused]] Iterator(bool first, const iterator1_t &iterator1, const iterator1_t &iterator1_end, const iterator2_t &iterator2):
                first(first), iterator1(iterator1), iterator1_end(iterator1_end), iterator2(iterator2) {}

        [[maybe_unused]] Iterator(const iterator1_t &iterator1, const iterator1_t &iterator1_end, const iterator2_t &iterator2):
                iterator1(iterator1), iterator1_end(iterator1_end), iterator2(iterator2) {
            first = iterator1 != iterator1_end;
        }

        [[maybe_unused]] reference operator *() const {
            return first ? *iterator1 : *iterator2;
        }

        [[maybe_unused]] Iterator operator ++() {
            if (first) {
                ++ iterator1;
                if (iterator1 == iterator1_end) {
                    first = false;
                }
            } else {
                ++ iterator2;
            }
            return *this;
        }

        [[maybe_unused]] bool operator ==(const Iterator &other) const {
            if (first != other.first) {
                return false;
            }
            return first ? iterator1 == other.iterator1 : iterator2 == other.iterator2;
        }

        [[maybe_unused]] bool operator !=(const Iterator &other) const {
            if (first == other.first) {
                return first ? iterator1 != other.iterator1 : iterator2 != other.iterator2;
            }
            return true;
        }
    };

    typedef typename std::conditional<be_const,
            Iterator<typename Range1::const_iterator, typename Range2::const_iterator>,
            Iterator<typename Range1::iterator, typename Range2::iterator>>::type iterator;
    typedef typename std::conditional<be_const,
            Iterator<typename Range1::const_reverse_iterator, typename Range2::const_reverse_iterator>,
            Iterator<typename Range1::reverse_iterator, typename Range2::reverse_iterator>>::type reverse_iterator;
    typedef Iterator<typename Range1::const_iterator, typename Range2::const_iterator> const_iterator;
    typedef Iterator<typename Range2::const_reverse_iterator, typename Range1::const_reverse_iterator> const_reverse_iterator;

    [[maybe_unused]] JoinedRange(Range1 &range1, Range2 &range2): range1(range1), range2(range2) {}

    [[maybe_unused]] [[nodiscard]] iterator begin() {
        return iterator(range1.begin(), range1.end(), range2.begin());
    }

    [[maybe_unused]] [[nodiscard]] iterator end() {
        return iterator(false, range1.end(), range1.end(), range2.end());
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rbegin() {
        return reverse_iterator(range2.rbegin(), range2.rend(), range1.rbegin());
    }

    [[maybe_unused]] [[nodiscard]] reverse_iterator rend() {
        return reverse_iterator(false, range2.rend(), range2.rend(), range1.rend());
    }

    [[maybe_unused]] [[nodiscard]] const_iterator begin() const {
        return const_iterator(range1.begin(), range1.end(), range2.begin());
    }

    [[maybe_unused]] [[nodiscard]] const_iterator end() const {
        return const_iterator(false, range1.end(), range1.end(), range2.end());
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rbegin() const {
        return const_reverse_iterator(range2.rbegin(), range2.rend(), range1.rbegin());
    }

    [[maybe_unused]] [[nodiscard]] const_reverse_iterator rend() const {
        return const_reverse_iterator(false, range2.rend(), range2.rend(), range1.rend());
    }
};


/// Return a joined range (1st range: left-value reference, 2nd range: left-value reference)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] JoinedRange<Range1, Range2> join(Range1 &range1, Range2 &range2) {
    return JoinedRange<Range1, Range2>(range1, range2);
}


/// Return a joined range (1st range: left-value reference, 2nd range: right-value)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] JoinedRange<Range1, Range2> join(Range1 &range1, Range2 &&range2) {
    return JoinedRange<Range1, Range2>(range1, range2);
}


/// Return a joined range (1st range: left-value reference, 2nd range: const reference)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] const JoinedRange<Range1, const Range2> join(Range1 &range1, const Range2 &range2) {
    return JoinedRange<Range1, const Range2>(range1, range2);
}


/// Return a joined range (1st range: right-value, 2nd range: left-value reference)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] JoinedRange<Range1, Range2> join(Range1 &&range1, Range2 &range2) {
    return JoinedRange<Range1, Range2>(range1, range2);
}


/// Return a joined range (1st range: right-value, 2nd range: right-value)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] JoinedRange<Range1, Range2> join(Range1 &&range1, Range2 &&range2) {
    return JoinedRange<Range1, Range2>(range1, range2);
}


/// Return a joined range (1st range: right-value, 2nd range: const reference)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] const JoinedRange<Range1, const Range2> join(Range1 &&range1, const Range2 &range2) {
    return JoinedRange<Range1, const Range2>(range1, range2);
}


/// Return a joined range (1st range: const reference, 2nd range: left-value reference)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] const JoinedRange<const Range1, Range2> join(const Range1 &range1, Range2 &range2) {
    return JoinedRange<const Range1, Range2>(range1, range2);
}


/// Return a joined range (1st range: const reference, 2nd range: right-value)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] const JoinedRange<const Range1, Range2> join(const Range1 &range1, Range2 &&range2) {
    return JoinedRange<const Range1, Range2>(range1, range2);
}


/// Return a joined range (1st range: const reference, 2nd range: const reference)
template <typename Range1, typename Range2>
[[maybe_unused]] [[nodiscard]] const JoinedRange<const Range1, const Range2> join(const Range1 &range1, const Range2 &range2) {
    return JoinedRange<const Range1, const Range2>(range1, range2);
}


/// Concat two ranges into a `std::vector`
template <typename Range1, typename Range2, typename value_type = typename Range1::value_type>
[[maybe_unused]] [[nodiscard]] std::vector<value_type> concat(const Range1 &range1, const Range2 &range2) {
    static_assert(std::is_same<typename Range1::value_type, typename Range2::value_type>::value,
                  "The types of two ranges in function concat must be same");
    std::vector<value_type> vec;
    for (const auto &value: range1) {
        vec.push_back(value);
    }
    for (const auto &value: range2) {
        vec.push_back(value);
    }
    return vec;
}


/// Map all the items in range into another `std::vector`
template <typename Range, typename Function>
[[maybe_unused]] [[nodiscard]] auto map(const Range &range, const Function &f) {
    std::vector<decltype(f(*range.begin()))> mapped;
    for (const auto &item: range) {
        mapped.push_back(f(item));
    }
    return mapped;
}


/// For each all the items in range (const reference)
template <typename Range, typename Function>
[[maybe_unused]] void for_each(const Range &range, const Function &f) {
    for (const auto &item: range) {
        f(item);
    }
}


/// For each all the items in range (const reference)
template <typename Range, typename Function>
[[maybe_unused]] void for_each(Range &range, const Function &f) {
    for (auto &item: range) {
        f(item);
    }
}


/// For each all the items in range (right-value)
template <typename Range, typename Function>
[[maybe_unused]] void for_each(Range &&range, const Function &f) {
    for (auto &item: range) {
        f(item);
    }
}


/// All of the items in the range satisfy the function
template <typename Range, typename Function>
[[maybe_unused]] bool all_of(const Range &range, const Function &f) {
    static_assert(std::is_same<bool, decltype(f(*range.begin()))>::value,
                  "The return type of function f must be bool");
    for (const auto &item: range) { // NOLINT(readability-use-anyofallof)
        if (not f(item)) {
            return false;
        }
    }
    return true;
}


/// None of the items in the range satisfies the function
template <typename Range, typename Function>
[[maybe_unused]] bool none_of(const Range &range, const Function &f) {
    static_assert(std::is_same<bool, decltype(f(*range.begin()))>::value,
                  "The return type of function f must be bool");
    for (const auto &item: range) { // NOLINT(readability-use-anyofallof)
        if (f(item)) {
            return false;
        }
    }
    return true;
}


/// Any of the items in the range satisfies the function
template <typename Range, typename Function>
[[maybe_unused]] bool any_of(const Range &range, const Function &f) {
    static_assert(std::is_same<bool, decltype(f(*range.begin()))>::value,
                  "The return type of function f must be bool");
    for (const auto &item: range) { // NOLINT(readability-use-anyofallof)
        if (f(item)) {
            return true;
        }
    }
    return false;
}


/// Find a certain item in a range
template <typename Range, typename value_type = typename Range::value_type>
[[maybe_unused]] [[nodiscard]] bool find(const Range &range, const value_type &value) {
    for (const auto &item: range) { // NOLINT(readability-use-anyofallof)
        if (item == value) {
            return true;
        }
    }
    return false;
}


/// Sum of all the values in a range
template <typename Range, typename value_type = typename Range::value_type>
[[maybe_unused]] [[nodiscard]] value_type sum(const Range &range) {
    value_type sum_value = 0;
    for (const auto &item: range) {
        sum_value += item;
    }
    return sum_value;
}


/// Check whether there are two duplicate items in a range
template <typename Range, typename value_type = typename Range::value_type>
[[maybe_unused]] [[nodiscard]] bool check_duplicate(const Range &range) {
    std::set<value_type> set;
    int size = 0;
    for (const auto &item: range) {
        set.insert(item);
        ++ size;
    }
    return set.size() != size;
}


/// Convert a number to `std::string` with units
template <typename T>
[[maybe_unused]] [[nodiscard]] std::string pretty(T value, T scale, const char* *units, int max_level) {
    int count = 0;
    auto d = static_cast<double>(value);
    while (d > scale && count < max_level - 1) {
        d /= scale;
        count += 1;
    }
    static char buffer[64];
    sprintf(buffer, "%.6f %s", d, units[count]);
    return buffer;
}


/// Convert a size to `std::string` with units
[[maybe_unused]] [[nodiscard]] std::string pretty_bytes(size_t size) {
    static const char* units[5] = {"B", "KiB", "MiB", "GiB"};
    return pretty<size_t>(size, 1024, units, 4);
}


/// Convert a nanosecond to `std::string` with units (always millisecond if `fixed` is true)
[[maybe_unused]] [[nodiscard]] std::string pretty_nanoseconds(uint64_t duration, bool fixed= true) {
    // To millisecond
    if (fixed) {
        static char buffer[64];
        sprintf(buffer, "%.6f ms", duration / 1e6);
        return buffer;
    }
    // Non-fixed
    static const char* units[5] = {"ns", "us", "ms", "s"};
    return pretty<uint64_t>(duration, 1000, units, 4);
}


/// Console colors
class [[maybe_unused]] ConsoleColor {
public:
    [[maybe_unused]] static constexpr const char *reset  = "\033[0m";
    [[maybe_unused]] static constexpr const char *black  = "\033[30m";
    [[maybe_unused]] static constexpr const char *red    = "\033[31m";
    [[maybe_unused]] static constexpr const char *green  = "\033[32m";
    [[maybe_unused]] static constexpr const char *yellow = "\033[33m";
    [[maybe_unused]] static constexpr const char *blue   = "\033[34m";
    [[maybe_unused]] static constexpr const char *white  = "\033[37m";
};


/// Print args (at least one)
template <typename Arg, typename... Args>
[[maybe_unused]] void print_args(Arg arg, Args... args) {
    std::cout << arg;
    if constexpr (sizeof...(Args) > 0) {
        std::cout << " ";
        print_args(args...);
    }
}


/// Debug print (file and line)
template <typename... Args>
[[maybe_unused]] void debug_print_impl(int line, const char *path, Args... args) {
    std::cout << ConsoleColor::green;
    std::cout << "[♫ Debug" << "#" << line << "@" << std::filesystem::path(path).filename().string() << "] " << ConsoleColor::reset;
    if constexpr (sizeof...(Args) > 0) {
        print_args(args...);
    } else {
        std::cout << "๑¯◡¯๑";
    }
    std::cout << std::endl;
}


/// Debug print (file and line, macro)
#define debug_print(...) debug_print_impl(__LINE__, __FILE__, ##__VA_ARGS__)


/// An unimplemented error raiser
[[maybe_unused]] void unimplemented_impl(int line, const char *file) {
    std::cerr << "Unimplemented part at line " << line << " in file " << file << std::endl;
    std::exit(EXIT_FAILURE);
}


/// An unimplemented error raiser (macro)
#define unimplemented() unimplemented_impl(__LINE__, __FILE__)


/// Size and time units' helper
class [[maybe_unused]] Unit {
public:
    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr size_t B(T size) {
        return size;
    }

    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr size_t KiB(T size) {
        return size * 1024ull;
    }

    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr size_t MiB(T size) {
        return size * 1024ull * 1024ull;
    }

    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr size_t GiB(T size) {
        return size * 1024ull * 1024ull * 1024ull;
    }

    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr uint64_t ns(T duration) {
        return duration;
    }

    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr uint64_t us(T duration) {
        return duration * 1000ull;
    }

    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr uint64_t ms(T duration) {
        return duration * 1000000ull;
    }

    template<typename T>
    [[maybe_unused]] [[nodiscard]] static constexpr uint64_t s(T duration) {
        return duration * 1000000000ull;
    }

    /// Convert `std::string` to `size_t`
    [[maybe_unused]] [[nodiscard]] static size_t bytes_from(const std::string &text) {
        const char *ptr = text.c_str();
        char *unit_ptr;
        double size = strtod(ptr, &unit_ptr);
        if (unit_ptr - ptr == text.size()) {
            std::cerr << "No unit specified" << std::endl;
        } else if (*unit_ptr == 'B') {
            return B(size);
        } else if (*unit_ptr == 'K') {
            return KiB(size);
        } else if (*unit_ptr == 'M') {
            return MiB(size);
        } else if (*unit_ptr == 'G') {
            return GiB(size);
        }
        std::cerr << "Failed to parse size (format: {num}{B/KiB/MiB/GiB}, e.g. 8GiB)" << std::endl;
        return 0;
    }
};


/// Dynamic bitset
class [[maybe_unused]] Bitset {
private:
    typedef uint64_t data_t;
    int bits = 0, data_length = 0;
    data_t *data = nullptr;
    bool hash_calculated = false;
    uint64_t hash_value = 0;

    [[maybe_unused]] void allocate() {
        data_length = bits;
        data_length /= width;
        data_length += bits % width == 0 ? 0 : 1;
        assert(data_length > 0);
        assert(data == nullptr);
        data = static_cast<data_t*> (std::malloc(data_length * width));
    }

public:
    static constexpr size_t width = sizeof(data_t) << 3;

    [[maybe_unused]] explicit Bitset(int bits): bits(bits) {
        allocate();
        clear();
    }

    [[maybe_unused]] Bitset(const Bitset &bitset) {
        bits = bitset.bits, data_length = bitset.data_length;
        data = static_cast<data_t*> (std::malloc(data_length * width));
        std::memcpy(data, bitset.data, data_length * width);
        hash_calculated = bitset.hash_calculated;
        hash_value = bitset.hash_value;
    }

    [[maybe_unused]] Bitset(int bits, const std::vector<int> &indexes): bits(bits) {
        allocate();
        clear();
        for (const auto &index: indexes) {
            set_bit(index, true);
        }
    }

    [[maybe_unused]] ~Bitset() {
        assert(data != nullptr);
        std::free(data);
    }

    /// Clear all the bits
    [[maybe_unused]] void clear() {
        std::memset(data, 0, data_length * width);
    }

    /// Check whether all the bits at indexes are 1
    [[maybe_unused]] [[nodiscard]] bool contains(const std::vector<int> &indexes) const {
        return all_of(indexes, [this](int index) -> bool {
            return get_bit(index);
        });
    }

    /// Set the bit at `index` to `bit`
    [[maybe_unused]] void set_bit(int index, bool bit) {
        // TODO: maybe add a `BitReference` to achieve `[]` operator
        assert(index >= 0 and index < bits);
        size_t i = index / width;
        size_t s = index % width;
        data[i] |= static_cast<data_t> (1) << s;
        data[i] ^= static_cast<data_t> (1) << s;
        data[i] |= static_cast<data_t> (bit) << s;
        hash_calculated = false;
    }

    /// Get the bit at `index`
    [[maybe_unused]] [[nodiscard]] bool get_bit(int index) const {
        assert(index >= 0 and index < bits);
        size_t i = index / width;
        size_t s = index % width;
        return (data[i] >> s) & static_cast<data_t> (1);
    }

    /// Get the hash value of the bitset
    [[maybe_unused]] [[nodiscard]] uint64_t hash() {
        if (not hash_calculated) {
            hash_calculated = true;
            hash_value = 0;
            for (int i = 0; i < data_length; ++ i) {
                hash_value = hash_value * 133ull + data[i];
            }
        }
        return hash_value;
    }
};