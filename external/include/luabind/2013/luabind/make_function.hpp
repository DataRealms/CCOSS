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

#ifndef LUABIND_MAKE_FUNCTION_060124_HPP
# define LUABIND_MAKE_FUNCTION_060124_HPP

# include <luabind/config.hpp>
# include <luabind/object.hpp>
# include <luabind/mpl_alias.hpp>
# include <luabind/aux_/function_call.hpp>
# include <luabind/aux_/function_rank.hpp>
# include <luabind/aux_/get_signature.hpp>

# include <boost/type_traits/is_member_function_pointer.hpp>
# include <boost/mem_fn.hpp>
# include <boost/function.hpp>

namespace luabind {

namespace aux
{
 
  typedef boost::function1<int, lua_State*> callback_type;
  
  LUABIND_API object make_function(
      lua_State* L, callback_type const& call, callback_type const& rank);

  template <class F, class Signature, class Policies>
  object make_function(
      lua_State* L, F fn, Signature, Policies const& policies, mpl::true_)
  {
      return make_function(
          L, boost::mem_fn(fn), Signature(), policies, mpl::false_()
      );
  }

  template <class F, class Signature, class Policies>
  object make_function(
      lua_State* L, F fn, Signature, Policies const& policies, mpl::false_)
  {
      return make_function(
          L
        , function_call<F, Signature, Policies>(fn, policies)
        , function_rank<Signature, Policies>(policies)
      );
  }

} // namespace aux

template <class F, class Signature, class Policies>
object make_function(lua_State* L, F fn, Signature, Policies const& policies)
{
    return aux::make_function(
        L, fn, Signature(), policies, boost::is_member_function_pointer<F>()
    );
}

template <class F>
object make_function(lua_State* L, F fn)
{
    return make_function(
        L, fn, aux::get_signature(fn), detail::null_type()
    );
}

LUABIND_API void add_overload(
    object const& target, char const* name, object const& fn);

} // namespace luabind

#endif // LUABIND_MAKE_FUNCTION_060124_HPP

