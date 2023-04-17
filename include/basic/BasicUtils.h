#ifndef BASIC_UTILS_H
#define BASIC_UTILS_H

#include <vector>
#include <numeric>

namespace momo{

template<typename T,std::integer_sequence<size_t,size_t... exp>>//power(T{},std::make_sequence<8>{})
constexpr T power(T base){
    T ret = 1;
    ((ret *= base,exp),...);
    for(int i = 0; i < exp; ++i){
        ret *= base;
    }
    return ret;
}



template<typename T, T... ints>
T sequence_product(std::integer_sequence<T, ints...> int_seq){
    T ret = 1;
    ((ret *= ints),...);
    return ret;
}

template<typename T>
constexpr T power(T base, int exp) {
    if constexpr(exp == 1){
        return base;
    }else{
        return base * power(base, exp - 1);
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
auto from_struct_to_reference_tuple(T&){
    static_assert(dependent_false_v<T>, "need a definition for `auto from_struct_to_tuple(T&)`");
    return T{};
}


}//momo


#endif
