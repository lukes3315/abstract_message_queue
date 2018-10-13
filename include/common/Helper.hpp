#ifndef HELPER_HPP
#define HELPER_HPP

#include <boost/variant.hpp>

namespace Helper{
    template<int Index, class Search, class First, class... Types>
    struct get_internal
    {
        typedef typename get_internal<Index + 1, Search, Types...>::type type;
        static constexpr int index = Index;
    };

    template<int Index, class Search, class... Types>
    struct get_internal<Index, Search, Search, Types...>
    {
        typedef get_internal type;
        static constexpr int index = Index;
    };

    template<class T, class... Types>
    int get_index()
    {
        return get_internal<0,T,Types...>::type::index;
    }
    
    template<typename ... data_types>
    struct visitor_pattern : public boost::static_visitor<void>
    {
        char* data_{nullptr};
        std::tuple<std::function<void(data_types & msg_data)>...> callbacks_;

        visitor_pattern(char * data, const std::tuple<std::function<void(data_types & msg_data)>...> & calls):data_(data),callbacks_(calls)
        {}
        template <typename T>
        void operator () (T && func) const
        {
            auto res = func(data_);
            std::get<std::function<void(decltype(res) &)> >(callbacks_)(res);
        }
    };

};

#endif