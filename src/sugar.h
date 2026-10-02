#include <string>
#include <print>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <variant>

using String = std::string;
#define print(...) std::println(__VA_ARGS__)

template<typename T>
using Vec = std::vector<T>;

template<typename T>
using Opt = std::optional<T>;

template<typename K,typename V>
using HashMap = std::unordered_map<K,V>;

using Byte = std::byte;

template<typename T>
using HashSet = std::unordered_set<T>;

template<typename T>
using Stack = std::stack<T>;

template<typename T>
using Deq = std::deque<T>;

#define Variant std::variant