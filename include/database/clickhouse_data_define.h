#ifndef CLICKHOUSE_DATA_DEFINE
#define CLICKHOUSE_DATA_DEFINE

namespace momo{
namespace clickhouse_utils{

template<size_t prec>
struct DateTime64{
    uint64_t v;
    uint64_t epoch_second(){
        return v / power(10, prec);
    }
};



}//clickhouse_utils
}//momo


#endif
