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

# ifndef LUABIND_FUNCTION_CALL_060125_HPP
#  define LUABIND_FUNCTION_CALL_060125_HPP

#  include <luabind/mpl_alias.hpp>
#  include <luabind/detail/policy.hpp>

#  include <boost/bind.hpp>
#  include <boost/mpl/long.hpp>
#  include <boost/mpl/bool.hpp>
#  include <boost/mpl/size.hpp>
#  include <boost/mpl/begin.hpp>
#  include <boost/mpl/deref.hpp>
#  include <boost/mpl/identity.hpp>
#  include <boost/mpl/if.hpp>

#  include <boost/type_traits/is_void.hpp>

#  include <boost/preprocessor/iteration/iterate.hpp>
#  include <boost/preprocessor/arithmetic/inc.hpp>
#  include <boost/preprocessor/iteration/local.hpp>
#  include <boost/preprocessor/cat.hpp>
#  include <boost/preprocessor/repetition/enum_trailing.hpp>

namespace luabind { namespace aux {

template <class F, class Signature, class Policies>
struct function_call
{
    function_call(F fn, Policies const& policies)
      : m_fn(fn)
      , m_policies(policies)
    {}

    int operator()(lua_State* L)
    {
        typedef mpl::long_<mpl::size<Signature>::value> arity;
        return function_call_impl(L, m_fn, Signature(), m_policies, arity());
    }

    F m_fn;
    Policies m_policies;
};

struct void_result
{};

template <class T, class Policies>
struct get_result_converter
  : mpl::apply_wrap2<
        typename detail::find_conversion_policy<0, Policies>::type
      , T
      , detail::cpp_to_lua
    >
{};

template <class T, class Policies>
struct result_converter
  : mpl::if_<
        boost::is_void<T>
      , mpl::identity<void_result>
      , get_result_converter<T, Policies>
    >::type
{};

template <class F, class RC>
void invoke(lua_State* L, F const& fn, RC& rc)
{
    rc.apply(L, fn());
}

template <class F>
void invoke(lua_State*, F const& fn, void_result)
{
    fn();
}

template <class T>
boost::reference_wrapper<T> make_reference_wrapper(T& x)
{
    return boost::reference_wrapper<T>(x);
}

template <class T>
boost::reference_wrapper<T const> make_reference_wrapper(T const& x)
{
    return boost::reference_wrapper<T const>(x);
}

#  define BOOST_PP_ITERATION_PARAMS_1 ( \
    3, (0, 5, <luabind/aux_/function_call.hpp>))
#  include BOOST_PP_ITERATE()

}} // namespace luabind::aux

# endif // LUABIND_FUNCTION_CALL_060125_HPP

#else

# define N BOOST_PP_ITERATION()

template <class F, class Signature, class Policies>
int function_call_impl(
    lua_State* L, F& fn, Signature, Policies const& policies, mpl::long_<N + 1>)
{
    typedef typename mpl::begin<Signature>::type iter0;
    typedef typename mpl::deref<iter0>::type result_type;

    typename result_converter<result_type, Policies>::type rc;

# if N > 0
#  define BOOST_PP_LOCAL_MACRO(n) \
    typedef typename mpl::next< \
        BOOST_PP_CAT(iter,n) \
    >::type BOOST_PP_CAT(iter, BOOST_PP_INC(n)); \
    typedef typename mpl::deref< \
        BOOST_PP_CAT(iter, BOOST_PP_INC(n)) \
    >::type BOOST_PP_CAT(arg, n); \
    typename mpl::apply_wrap2< \
        typename detail::find_conversion_policy<BOOST_PP_INC(n), Policies>::type \
      , BOOST_PP_CAT(arg, n) \
      , detail::lua_to_cpp \
    >::type BOOST_PP_CAT(c,n);

#  define BOOST_PP_LOCAL_LIMITS (0, N - 1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif

    int const top = lua_gettop(L);

# define LUABIND_GET_ARG(z, n, _) \
    make_reference_wrapper( \
        BOOST_PP_CAT(c,n).apply( \
            L, LUABIND_DECORATE_TYPE(BOOST_PP_CAT(arg,n)), BOOST_PP_INC(n) \
        ) \
    ) \

    invoke(
        L
      , boost::bind<result_type>(
            boost::ref(fn)
            BOOST_PP_ENUM_TRAILING(N, LUABIND_GET_ARG, ~)
        )
      , rc
    );

# undef LUABIND_GET_ARG

    return lua_gettop(L) - top;
}

# undef N

#endif

