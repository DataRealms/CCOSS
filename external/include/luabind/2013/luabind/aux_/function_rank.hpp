// Copyright (c) 2006 Daniel Wallin

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#if !BOOST_PP_IS_ITERATING

# ifndef LUABIND_FUNCTION_RANK_060125_HPP
#  define LUABIND_FUNCTION_RANK_060125_HPP

#  include <luabind/mpl_alias.hpp>
#  include <luabind/detail/policy.hpp>

#  include <boost/mpl/long.hpp>
#  include <boost/mpl/begin_end.hpp>
#  include <boost/mpl/deref.hpp>
#  include <boost/mpl/next.hpp>

namespace luabind { namespace aux {

template <class Iterator, class End, class Index, class Policies>
int calculate_rank(lua_State* L, Iterator, End, Index, Policies const& policies)
{
    typedef typename mpl::deref<Iterator>::type argument;
    
    typedef typename mpl::apply_wrap2<
        typename detail::find_conversion_policy<Index::value, Policies>::type
      , argument
      , detail::lua_to_cpp
    >::type cv; 

    int rank = cv::match(
        L, LUABIND_DECORATE_TYPE(argument), Index::value
    );

    if (rank < 0) return -1;

    int next = calculate_rank(
        L
      , typename mpl::next<Iterator>::type()
      , End()
      , typename mpl::next<Index>::type()
      , policies
    );

    if (next < 0) return -1;

    return rank + next;
}

template <class End, class Index, class Policies>
int calculate_rank(lua_State*, End, End, Index, Policies const&)
{
    return 0;
}

template <class Signature, class Policies>
struct function_rank
{
    function_rank(Policies const& policies)
      : m_policies(policies)
    {}

    int operator()(lua_State* L)
    {
        if (lua_gettop(L) != mpl::size<Signature>::value - 1)
            return -1;

        return calculate_rank(
            L
          , typename mpl::next<typename mpl::begin<Signature>::type>::type()
          , typename mpl::end<Signature>::type()
          , mpl::long_<1>()
          , m_policies
        );
    }

    Policies m_policies;
};
/*
#  define BOOST_PP_ITERATION_PARAMS_1 ( \
    3, (0, 5, <luabind/aux_/function_rank.hpp>))
#  include BOOST_PP_ITERATE()*/

}} // namespace luabind::aux

# endif // LUABIND_FUNCTION_RANK_060125_HPP

#else

# define N BOOST_PP_ITERATION()

#endif

