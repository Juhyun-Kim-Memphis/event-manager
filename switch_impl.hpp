#ifdef BOOST_PP_IS_ITERATING

#define NUMBER_OF_CASES BOOST_PP_FRAME_ITERATION(1)

template <typename Xs>
struct switch_impl<NUMBER_OF_CASES, Xs> {
#   define TYPEDEF_X_SUB_I(_, i, __) \
      typedef typename boost::mpl::at_c<Xs, i>::type x_##i;
    BOOST_PP_REPEAT(NUMBER_OF_CASES, TYPEDEF_X_SUB_I, ~)

    typedef typename boost::mpl::at_c<Xs, NUMBER_OF_CASES>::type x_default;

#   define METALEVEL_CASE_TABLE_ENTRY(_, i, ts) \
      case boost::mpl::first<x_##i>::type::value: \
        return boost::mpl::second<x_##i>::type::value ts ;

#   define METALEVEL_DEFAULT_TABLE_ENTRY(ts) \
        default: \
            return boost::mpl::second<x_default>::type::value ts ;

#   define DEFINE_CHOICE_RUN(_, argc, __) \
      template <typename R BOOST_PP_COMMA_IF(argc) \
                BOOST_PP_ENUM_PARAMS(argc, typename T)> \
      static always_inline R \
      run(int n BOOST_PP_COMMA_IF(argc) \
          BOOST_PP_ENUM_BINARY_PARAMS(argc, T, t)) { \
          switch(n) { \
          BOOST_PP_REPEAT(NUMBER_OF_CASES, \
                          METALEVEL_CASE_TABLE_ENTRY, \
                          (BOOST_PP_ENUM_PARAMS(argc, t))) \
          METALEVEL_DEFAULT_TABLE_ENTRY((BOOST_PP_ENUM_PARAMS(argc, t))) \
          } \
      }
    BOOST_PP_REPEAT(METALEVEL_MAX_CHOICE_ARGS,
                    DEFINE_CHOICE_RUN, ~)

#   undef DEFINE_CHOICE_RUN
#   undef METALEVEL_CASE_TABLE_ENTRY
#   undef METALEVEL_TABLE_ENTRY
};

#undef NUMBER_OF_CASES

#endif
