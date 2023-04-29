#ifndef BASIC_UTILS_H
#define BASIC_UTILS_H

#include <vector>
#include <numeric>
#include <map>
#include <functional>

namespace momo{

template <typename K, typename V>
void map_diff_op(const std::map<K, V> &new_map, const std::map<K, V> &old_map,
            std::function<void (const K &key, const V &value)> add,
            std::function<void (const K &key, const V &new_value, const V &old_value)> change = nullptr,
            std::function<void (const K &key, const V &value)> del = nullptr,
            std::function<void (const K &key, const V &value)> eq = nullptr,
            std::function<bool (const V &lhs, const V &rhs)> op = [](const V &lhs, const V &rhs) -> bool {
                                                                        return lhs == rhs;
                                                                    })
{
    auto it_new = new_map.begin();
    auto it_old = old_map.begin();
    auto it_new_end = new_map.end();
    auto it_old_end = old_map.end();
    while (it_old != it_old_end && it_new != it_new_end) {
        if (it_old->first == it_new->first) {
            if (eq || change) {
                if (op(it_new->second, it_old->second)) {
                    if (eq) {
                        eq(it_new->first, it_new->second);
                    }
                } else {
                    if (change) {
                        change(it_new->first, it_new->second, it_old->second);
                    }
                }
            }
            ++it_old;
            ++it_new;
        } else if (it_old->first < it_new->first) {
            if (del) {
                del(it_old->first, it_old->second);
            }
            ++it_old;
        } else {
            if (add) {
                add(it_new->first, it_new->second);
            }
            ++it_new;
        }
    }

    while (it_old != old_map.end()) {
        if (del) {
            del(it_old->first, it_old->second);
        }
        ++it_old;
    }

    while (it_new != new_map.end()) {
        if (add) {
            add(it_new->first, it_new->second);
        }
        ++it_new;
    }
}


template<typename T>
std::vector<T> expand(const std::vector<std::vector<T>> &src) {
    std::vector<T> dst;
    int total = std::accumulate(src.begin(), src.end(), 0, 
        [](auto acc, auto const &part){
            return acc + part.size();
        }
    );
    dst.reserve(total);
    std::for_each(src.begin(), src.end(),
        [&dst](auto const &part){
            std::copy(part.begin(), part.end(), std::back_inserter(dst));
        }
    );
    return std::move(dst);
}

template<class>
inline constexpr bool dependent_false_v = false;

template<typename T>
auto from_struct_to_reference_tuple(T&) {
    static_assert(dependent_false_v<T>, "need a definition for `auto from_struct_to_tuple(T&)`");
    return T{};
}


}//momo


#endif
