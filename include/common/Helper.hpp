#ifndef HELPER_HPP
#define HELPER_HPP

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
};

#endif