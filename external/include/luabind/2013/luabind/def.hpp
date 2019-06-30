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

#ifndef LUABIND_DEF_060126_HPP
# define LUABIND_DEF_060126_HPP

# include <luabind/scope.hpp>
# include <luabind/make_function.hpp>

namespace luabind {

namespace aux
{

  template <class F, class Policies>
  struct def_registration : detail::registration
  {
      def_registration(char const* name, F const& fn, Policies const& policies)
        : m_name(name)
        , m_fn(fn)
        , m_policies(policies)
      {}

      void register_(lua_State* L) const
      {
          add_overload(
              object(from_stack(L, -1))
            , m_name
            , luabind::make_function(
                  L
                , m_fn
                , get_signature(m_fn)
                , m_policies
              )
          );
      }

  private:
      char const* m_name;
      F m_fn;
      Policies const& m_policies;
  };

} // namespace aux

template <class F, class Policies>
scope def(char const* name, F fn, Policies const& policies)
{
    return scope(
        std::auto_ptr<detail::registration>(
            new aux::def_registration<F, Policies>(name, fn, policies)
        )
    );
}

template <class F>
scope def(char const* name, F fn)
{
    return def(name, fn, detail::null_type());
}

} // namespace luabind

#endif // LUABIND_DEF_060126_HPP

