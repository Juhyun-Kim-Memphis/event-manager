#ifndef metalevel_switch_hpp
#define metalevel_switch_hpp

#include <boost/mpl/at.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/size.hpp>

#include <boost/preprocessor/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

#ifdef __GNUC__
#  define always_inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#  define always_inline __forceinline
#else
#  define always_inline inline
#endif

#ifndef METALEVEL_MAX_CHOICE_ARGS
#  define METALEVEL_MAX_CHOICE_ARGS 20
#endif

#ifndef METALEVEL_MAX_SWITCH_SIZE
#  define METALEVEL_MAX_SWITCH_SIZE 20
#endif

namespace metalevel {

    namespace aux {
        template <int NumberOfCases, typename Xs>
        struct switch_impl;
#   define BOOST_PP_ITERATION_LIMITS (0, METALEVEL_MAX_SWITCH_SIZE)
#   define BOOST_PP_FILENAME_1 <switch_impl.hpp>
#   include BOOST_PP_ITERATE()

        template <int Counter, int N, template <int> class Table>
        struct tabulate
                : boost::mpl::push_front<
                        tabulate<Counter+1, N-1, Table>,
                        boost::mpl::pair<boost::mpl::int_<Counter>, Table<Counter> >
                >::type {};

        template <int Counter, template <int> class Table>
        struct tabulate<Counter, 0, Table>
                : boost::mpl::list<
                        boost::mpl::pair<boost::mpl::int_<Counter>, Table<Counter> >
                > {};
    } // namespace aux

    template <typename Xs>
    struct switch_
            : aux::switch_impl<boost::mpl::size<Xs>::type::value-1, Xs> {};

    template <int Counter, int N, template <int> class Table, bool addDefault = true>
    struct switch_table
            : switch_<aux::tabulate<Counter,N+addDefault,Table> > {};
} // namespace metalevel


#endif