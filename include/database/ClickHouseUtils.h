#ifndef CLICKHOUSE_UTILS
#define CLICKHOUSE_UTILS
#include <string>
#include "ClickHouseDataDefine.h"
#include "hv/EventLoopThreadPool.h"
#include "clickhouse/client.h"
#include "ConcurrencyUtils.h"


/*
auto get_field_tuple_ref(T& t)
{
    if constexpr (std::is_fundamental_v<T> || is_same_v<std::string,T>)
    {
        return std::tie(t);
    }
    else
    {
        return from_struct_to_reference_tuple(t);
    }
}

auto total_tuple = std::tie();
std::apply([&total_tuple,&item,&index](auto&& ...args){
        (std::tuple_cat(total_tuple,get_field_tuple_ref(args)),...);
        },from_struct_to_reference_tuple(t));
*/
namespace momo{
namespace clickhouse_utils
{

using namespace clickhouse;

template<typename T>
auto get_field_tuple_ref(T& t){
    if constexpr (std::is_fundamental_v<T> || is_same_v<std::string,T> || is_same_v<DateTime64, T>){
        return std::tie(t);
    }else{
        return from_struct_to_reference_tuple(t);
    }
}

template<typename T>
void get_value(T &value, const clickhouse::Block &block, int row, int col){
    if constexpr(std::is_same<T, int32_t>::value){
        value = block[col]->As<ColumnInt32>()->At(row);
    }else if constexpr(std::is_same<T, uint32_t>::value){
        value = block[col]->As<ColumnUInt32>()->At(row);
    }else if constexpr(std::is_same<T, int64_t>::value){
        value = block[col]->As<ColumnInt64>()->At(row);
    }else if constexpr(std::is_same<T, uint64_t>::value){
        value = block[col]->As<ColumnUInt64>()->At(row);
    }else if constexpr(std::is_same<std::string,T>::value){
        value = std::string(block[col]->As<ColumnString>()->At(row));
    }else if constexpr(std::is_same<T, DateTime64>::value){
        value.v = block[col]->As<ColumnDateTime64>()->At(row);
    }else{
        static_assert(std::is_same_v<T, false_type>, "please add more type");
        value = T{};
    }
}

template<typename T>
auto get_column(T &value){
    if constexpr(std::is_same<T, int32_t>::value){
        return std::make_shared<ColumnInt32>();
    }else if constexpr(std::is_same<T, uint32_t>::value){
        return std::make_shared<ColumnUInt32>();
    }else if constexpr(std::is_same<T, int64_t>::value){
        return std::make_shared<ColumnInt64>();
    }else if constexpr(std::is_same<T, uint64_t>::value){
        return std::make_shared<ColumnUInt64>();
    }else if constexpr(std::is_same<T, std::string>::value){
        return std::make_shared<ColumnString>();
    }else if constexpr(std::is_same<T, DateTime64>::value){
        return std::make_shared<ColumnDateTime64>(3); //3 become to template?
    }else{
        static_assert(std::is_same_v<T, false_type>, "please add more type");
        return std::make_shared<int>();
    }
}

template<typename T>
void col_append(T &v, int idx, std::vector<std::shared_ptr<Column>> &vecColumn){
    Column *pCol = vecColumn[idx].get();
    if constexpr(std::is_same<T, int32_t>::value){
        static_cast<ColumnInt32 *>(pCol)->Append(v);
    }else if constexpr(std::is_same<T, uint32_t>::value){
        static_cast<ColumnUInt32 *>(pCol)->Append(v);
    }else if constexpr(std::is_same<T, int64_t>::value){
        static_cast<ColumnInt64 *>(pCol)->Append(v);
    }else if constexpr(std::is_same<T, uint64_t>::value){
        static_cast<ColumnUInt64 *>(pCol)->Append(v);
    }else if constexpr(std::is_same<T, std::string>::value){
        static_cast<ColumnString *>(pCol)->Append(v);
    }else if constexpr(std::is_same<T, DateTime64>::value){
        static_cast<ColumnDateTime64 *>(pCol)->Append(v.v);
    }else{
        static_assert(std::is_same<T, false_type>::value, "please add more type");
    }
}

template<typename T>
void check_lref(T&& t)
{
    static_assert(std::is_lvalue_reference_v<T>,"every element of the tuple should be lvalue_reference");
}

template<typename T>
vector<T> parse(const clickhouse::Block &block){
    vector<T> vec;
    int sz = block.GetRowCount();
    vec.reserve(sz);
    for(int row = 0; row < block.GetRowCount(); ++row){
        vec.emplace_back();
        T &t = vec.back();
        using clickhouse_utils::from_struct_to_reference_tuple;

        std::apply([](auto&& ...args){
            (check_lref(std::forward<decltype(args)>(args)),...);
        },from_struct_to_reference_tuple(t));

        int col = 0;
        std::apply([&block, &row, &col](auto&& ...args){
            (get_value(args, block, row, col++),...);
        },from_struct_to_reference_tuple(t));
    }
    return std::move(vec);
}

/*
template<typename T, size_t POOL_SIZE>
vector<T> parse_mt(const std::vector<clickhouse::Block> &blocks){
    static_assert(POOL_SIZE >= 2 && POOL_SIZE <= 8, "POOL_SIZE should between 2 and 8.");
    hv::EventLoopThreadPool pool(POOL_SIZE);
    pool.start();

    int blockCount = blocks.size();
    int totalItemCount = std::accumulate(blocks.begin(), blocks.end(), 0, [](auto sum, auto const &block){
        return sum + block.GetRowCount();
    });

    vector<T> vec;
    vec.reserve(totalItemCount);

    int itemIndex = 0;

    CountDownLatch latch(blockCount);

    for(int i = 0; i < blockCount; ++i){
        auto const &block = blocks[i];
        int threadIndex = i % POOL_SIZE;
        itemIndex += blocks.GetRowCount();

        //&block is ok?
        pool[threadIndex]->queueInLoop([itemIndex, &block, &vec](){
            vector<T> tmp = parse(block);
            auto itDst = vec.begin() + itemIndex;
            std::copy(tmp.begin(), tmp.end(), itDst);
            latch.countdown();
        });
    }
    latch.wait();
    pool.stop();
    pool.join();

    return std::move(vec);
}
*/

/*
template<typename T, size_t POOL_SIZE>
vector<T> parse_mt(const std::vector<clickhouse::Block> &blocks){
    static_assert(POOL_SIZE >= 2 && POOL_SIZE <= 8, "POOL_SIZE should between 2 and 8.");
    hv::EventLoopThreadPool pool(POOL_SIZE);
    pool.start();

    int blockCount = blocks.size();

    vector<int> vecWriteIndex;
    vecWriteIndex.reserve(blockCount);
    int totalItemCount = std::accumulate(blocks.begin(), blocks.end(), 0, 
        [](auto sum, auto const &block){
            vecWriteIndex.emplace_back(sum);
            return sum + block.GetRowCount();
        }
    );

    vector<T> vec;
    vec.resize(totalItemCount);
    
    CountDownLatch latch(POOL_SIZE);
    atomic_int incBlockIndex = 0;
    for(int i = 0; i < POOL_SIZE; ++i){
        pool[i]->queueInLoop([&vecWriteIndex, &blocks](){
            while(true){
                int blockIndex = incBlockIndex.fetch_add(1);
                if(blockIndex >= blockCount){
                    break;
                }
                vector<T> tmp = parse(block);
                auto writeIndex = vecWriteIndex[blockIndex];
                auto itDst = vec.begin() + writeIndex;
                std::copy(tmp.begin(), tmp.end(), itDst);
            }
            latch.countdown();
        });
    }
    latch.wait();
    return std::move(vec);
}
*/


template<typename T, size_t POOL_SIZE>
vector<T> parse_mt(const std::vector<clickhouse::Block> &blocks){
    vector<vector<T>> dst = convert_mt<T, clickhouse::Block, POOL_SIZE>(blocks,
        [](const auto &block){
            std::vector<T> tmp = parse(block);
            return std::move(tmp);
        }
    );
    vector<T> ret = expand(std::move(dst));
    return std::move(ret);
}


template<typename T>
clickhouse::Block to_block(const std::vector<T> &vecRecord){
    clickhouse::Block block;
    std::vector<std::shared_ptr<Column>> vecColumn;

    using clickhouse_utils::from_struct_to_reference_tuple;

    T t;
    auto tuple = from_struct_to_reference_tuple(t);

    int n = 0;
    std::apply([&vecColumn, &n](auto&&... args){
        ((vecColumn.push_back(get_column(args)), ++n),...);
    }, tuple);

    for(auto const& item : vecRecord){
        auto tuple = from_struct_to_reference_tuple(const_cast<T&>(item));
        int idx = 0;
        std::apply([&idx, &vecColumn](auto&&... args){
            (col_append(args, idx++, vecColumn),...);
        }, tuple);
    }

    for(int i = 0; i < n; ++i){
        block.AppendColumn(T::vecField[i], vecColumn[i]);
    }
    return std::move(block);
}

}//clickhouse_utils
}//momo

#endif
