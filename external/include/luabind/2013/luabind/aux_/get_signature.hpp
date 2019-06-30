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

# ifndef LUABIND_GET_SIGNATURE_060124_HPP
#  define LUABIND_GET_SIGNATURE_060124_HPP

#  include <luabind/mpl_alias.hpp>
#  include <luabind/detail/most_derived.hpp>
#  include <boost/mpl/vector.hpp>
#  include <boost/preprocessor/iteration/iterate.hpp>
#  include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#  include <boost/preprocessor/repetition/enum_params.hpp>
#  include <boost/preprocessor/arithmetic/inc.hpp>

namespace luabind { namespace aux {

#  define BOOST_PP_ITERATION_PARAMS_1 ( \
    4, (0, 2, <luabind/aux_/get_signature.hpp>, 0))
#  include BOOST_PP_ITERATE()

#  define BOOST_PP_ITERATION_PARAMS_1 ( \
    4, (0, 2, <luabind/aux_/get_signature.hpp>, 1))
#  include BOOST_PP_ITERATE()

#  define BOOST_PP_ITERATION_PARAMS_1 ( \
    4, (0, 2, <luabind/aux_/get_signature.hpp>, 2))
#  include BOOST_PP_ITERATE()
    
}} // namespace luabind::aux

# endif // LUABIND_GET_SIGNATURE_060124_HPP

#elif BOOST_PP_ITERATION_FLAGS() == 0

# define N BOOST_PP_ITERATION()
# define LUABIND_VECTOR(N) BOOST_PP_CAT(mpl::vector, BOOST_PP_INC(N))

template <class R BOOST_PP_ENUM_TRAILING_PARAMS(N, class A)>
LUABIND_VECTOR(N)<
    R BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
> get_signature(R(*)(BOOST_PP_ENUM_PARAMS(N, A)), ...)
{
    return LUABIND_VECTOR(N)<
        R BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
    >();
}

# undef LUABIND_VECTOR
# undef N

#else

# define N BOOST_PP_ITERATION()
# if BOOST_PP_ITERATION_FLAGS() == 1
#  define CQ
# else
#  define CQ const
# endif

# define LUABIND_VECTOR(N) BOOST_PP_CAT(mpl::vector, BOOST_PP_INC(BOOST_PP_INC(N)))

template <
    class R, class C BOOST_PP_ENUM_TRAILING_PARAMS(N, class A), class Wrapped
>
LUABIND_VECTOR(N)<
    R 
  , typename detail::most_derived<C, Wrapped>::type CQ&
    BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
> get_signature(R(C::*)(BOOST_PP_ENUM_PARAMS(N, A)) CQ, Wrapped*)
{
    return LUABIND_VECTOR(N)<
        R
      , typename detail::most_derived<C, Wrapped>::type CQ&
        BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
    >();
}

template <
    class R, class C BOOST_PP_ENUM_TRAILING_PARAMS(N, class A)
>
LUABIND_VECTOR(N)<
    R, C CQ& BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
> get_signature(R(C::*)(BOOST_PP_ENUM_PARAMS(N, A)) CQ)
{
    return LUABIND_VECTOR(N)<
        R, C CQ& BOOST_PP_ENUM_TRAILING_PARAMS(N, A)
    >();
}

# undef LUABIND_VECTOR
# undef CQ
# undef N

#endif

