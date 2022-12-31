// A Bison parser, made by GNU Bison 3.0.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.


// First part of user declarations.

#line 37 "src/generated/lang.tab.cc" // lalr1.cc:404

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

#include "lang.tab.hh"

// User implementation prologue.

#line 51 "src/generated/lang.tab.cc" // lalr1.cc:412
// Unqualified %code blocks.
#line 28 "src/lang.y" // lalr1.cc:413

	#include <iostream>
  #include <fstream>
  #include <string_view>
  
	extern int yylex(yy::parser::semantic_type* yylval, yy::parser::location_type* loc);

	using namespace npcompiler::ast;
	using namespace npcompiler;

	#define mkn(...) new AstNode(__VA_ARGS__)
	#define msv(x) std::string_view(x.ptr, x.len)

#line 67 "src/generated/lang.tab.cc" // lalr1.cc:413


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (/*CONSTCOND*/ false)
# endif


// Suppress unused-variable warnings by "using" E.
#define YYUSE(E) ((void) (E))

// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << std::endl;                  \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yystack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE(Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void>(0)
# define YY_STACK_PRINT()                static_cast<void>(0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
#line 153 "src/generated/lang.tab.cc" // lalr1.cc:479

  /* Return YYSTR after stripping away unnecessary quotes and
     backslashes, so that it's suitable for yyerror.  The heuristic is
     that double-quoting is unnecessary unless the string contains an
     apostrophe, a comma, or backslash (other than backslash-backslash).
     YYSTR is taken from yytname.  */
  std::string
  parser::yytnamerr_ (const char *yystr)
  {
    if (*yystr == '"')
      {
        std::string yyr = "";
        char const *yyp = yystr;

        for (;;)
          switch (*++yyp)
            {
            case '\'':
            case ',':
              goto do_not_strip_quotes;

            case '\\':
              if (*++yyp != '\\')
                goto do_not_strip_quotes;
              // Fall through.
            default:
              yyr += *yyp;
              break;

            case '"':
              return yyr;
            }
      do_not_strip_quotes: ;
      }

    return yystr;
  }


  /// Build a parser object.
  parser::parser (npcompiler::ast::ParserContext& ctx__yyarg, npcompiler::ast::AstNode& root__yyarg)
    :
#if YYDEBUG
      yydebug_ (false),
      yycdebug_ (&std::cerr),
#endif
      ctx_ (ctx__yyarg),
      root_ (root__yyarg)
  {}

  parser::~parser ()
  {}


  /*---------------.
  | Symbol types.  |
  `---------------*/

  inline
  parser::syntax_error::syntax_error (const location_type& l, const std::string& m)
    : std::runtime_error (m)
    , location (l)
  {}

  // basic_symbol.
  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol ()
    : value ()
  {}

  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& other)
    : Base (other)
    , value ()
    , location (other.location)
  {
    value = other.value;
  }


  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const semantic_type& v, const location_type& l)
    : Base (t)
    , value (v)
    , location (l)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  inline
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, const location_type& l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  inline
  parser::basic_symbol<Base>::~basic_symbol ()
  {
    clear ();
  }

  template <typename Base>
  inline
  void
  parser::basic_symbol<Base>::clear ()
  {
    Base::clear ();
  }

  template <typename Base>
  inline
  bool
  parser::basic_symbol<Base>::empty () const
  {
    return Base::type_get () == empty_symbol;
  }

  template <typename Base>
  inline
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move(s);
    value = s.value;
    location = s.location;
  }

  // by_type.
  inline
  parser::by_type::by_type ()
    : type (empty_symbol)
  {}

  inline
  parser::by_type::by_type (const by_type& other)
    : type (other.type)
  {}

  inline
  parser::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  inline
  void
  parser::by_type::clear ()
  {
    type = empty_symbol;
  }

  inline
  void
  parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
  }

  inline
  int
  parser::by_type::type_get () const
  {
    return type;
  }


  // by_state.
  inline
  parser::by_state::by_state ()
    : state (empty_state)
  {}

  inline
  parser::by_state::by_state (const by_state& other)
    : state (other.state)
  {}

  inline
  void
  parser::by_state::clear ()
  {
    state = empty_state;
  }

  inline
  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  inline
  parser::by_state::by_state (state_type s)
    : state (s)
  {}

  inline
  parser::symbol_number_type
  parser::by_state::type_get () const
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  inline
  parser::stack_symbol_type::stack_symbol_type ()
  {}


  inline
  parser::stack_symbol_type::stack_symbol_type (state_type s, symbol_type& that)
    : super_type (s, that.location)
  {
    value = that.value;
    // that is emptied.
    that.type = empty_symbol;
  }

  inline
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }


  template <typename Base>
  inline
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    YYUSE (yysym.type_get ());
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo,
                                     const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    symbol_number_type yytype = yysym.type_get ();
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  inline
  void
  parser::yypush_ (const char* m, state_type s, symbol_type& sym)
  {
    stack_symbol_type t (s, sym);
    yypush_ (m, t);
  }

  inline
  void
  parser::yypush_ (const char* m, stack_symbol_type& s)
  {
    if (m)
      YY_SYMBOL_PRINT (m, s);
    yystack_.push (s);
  }

  inline
  void
  parser::yypop_ (unsigned int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  inline parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  inline bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  inline bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::parse ()
  {
    // State.
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

    // FIXME: This shoud be completely indented.  It is not yet to
    // avoid gratuitous conflicts when merging into the master branch.
    try
      {
    YYCDEBUG << "Starting parse" << std::endl;


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, yyla);

    // A new symbol was pushed on the stack.
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << std::endl;

    // Accept?
    if (yystack_[0].state == yyfinal_)
      goto yyacceptlab;

    goto yybackup;

    // Backup.
  yybackup:

    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
        try
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
        catch (const syntax_error& yyexc)
          {
            error (yyexc);
            goto yyerrlab1;
          }
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.type_get ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.type_get ())
      goto yydefault;

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", yyn, yyla);
    goto yynewstate;

  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;

  /*-----------------------------.
  | yyreduce -- Do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_(yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Compute the default @$.
      {
        slice<stack_symbol_type, stack_type> slice (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, slice, yylen);
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
      try
        {
          switch (yyn)
            {
  case 2:
#line 79 "src/lang.y" // lalr1.cc:859
    { root_.push((yystack_[0].value.val_node)); (yylhs.value.val_node) = &root_; }
#line 634 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 3:
#line 80 "src/lang.y" // lalr1.cc:859
    { root_.push((yystack_[0].value.val_node)); (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 640 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 4:
#line 84 "src/lang.y" // lalr1.cc:859
    { 
			(yylhs.value.val_node) = (yystack_[2].value.val_node);
			(yystack_[2].value.val_node)->push((yystack_[1].value.val_node), mkn(AstType::Program_End)); 
			}
#line 649 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 5:
#line 88 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[2].value.val_node), (yystack_[2].value.val_node)->push((yystack_[1].value.val_node), mkn(AstType::Function_End)); }
#line 655 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 6:
#line 89 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 661 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 7:
#line 93 "src/lang.y" // lalr1.cc:859
    { ctx_.symbols_local.clear(); }
#line 667 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 8:
#line 97 "src/lang.y" // lalr1.cc:859
    { ctx_.symbols_local.clear(); }
#line 673 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 9:
#line 101 "src/lang.y" // lalr1.cc:859
    { ctx_.symbols_local.clear(); }
#line 679 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 10:
#line 105 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(AstType::Function, (yystack_[0].value.val_node)); }
#line 685 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 11:
#line 106 "src/lang.y" // lalr1.cc:859
    { (yystack_[1].value.val_node)->push((yystack_[0].value.val_node)); (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 691 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 12:
#line 110 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 697 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 13:
#line 114 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(AstType::StmtList); }
#line 703 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 14:
#line 115 "src/lang.y" // lalr1.cc:859
    { (yystack_[1].value.val_node)->push((yystack_[0].value.val_node)); (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 709 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 15:
#line 119 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 715 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 16:
#line 120 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 721 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 17:
#line 124 "src/lang.y" // lalr1.cc:859
    { ctx_.error = true; std::cerr << "missing semicolon after the statement.\n"; }
#line 727 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 19:
#line 128 "src/lang.y" // lalr1.cc:859
    { ctx_.error = true; std::cerr << "missing semicolon after the var declaration.\n"; }
#line 733 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 21:
#line 132 "src/lang.y" // lalr1.cc:859
    {(yylhs.value.val_node) = ctx_.ident_get(msv((yystack_[0].value.val_str)));}
#line 739 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 22:
#line 133 "src/lang.y" // lalr1.cc:859
    {(yylhs.value.val_node) = ctx_.ext_ident_get(msv((yystack_[0].value.val_str)));}
#line 745 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 23:
#line 137 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = ctx_.create_assignment((yystack_[3].value.val_node), (yystack_[1].value.val_node)); }
#line 751 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 24:
#line 141 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_BOOL; }
#line 757 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 25:
#line 142 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_U8; }
#line 763 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 26:
#line 143 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_I8; }
#line 769 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 27:
#line 144 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_U16; }
#line 775 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 28:
#line 145 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_I16; }
#line 781 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 29:
#line 146 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_U32; }
#line 787 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 30:
#line 147 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_I32; }
#line 793 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 31:
#line 148 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_int) = npsys::nptype::NPT_F32; }
#line 799 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 32:
#line 152 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(AstType::Program); }
#line 805 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 33:
#line 153 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); (yystack_[1].value.val_node)->push((yystack_[0].value.val_node)); }
#line 811 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 34:
#line 154 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); (yystack_[1].value.val_node)->push((yystack_[0].value.val_node)); }
#line 817 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 35:
#line 158 "src/lang.y" // lalr1.cc:859
    { ctx_.global = false; }
#line 823 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 36:
#line 158 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 829 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 37:
#line 162 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(AstType::LocalVarDeclSeq); }
#line 835 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 38:
#line 163 "src/lang.y" // lalr1.cc:859
    { (yystack_[1].value.val_node)->push((yystack_[0].value.val_node)); (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 841 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 39:
#line 167 "src/lang.y" // lalr1.cc:859
    { ctx_.global = true; }
#line 847 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 40:
#line 167 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 853 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 41:
#line 171 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(AstType::GlobalVarDeclSeq); }
#line 859 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 42:
#line 172 "src/lang.y" // lalr1.cc:859
    { (yystack_[1].value.val_node)->push((yystack_[0].value.val_node)); (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 865 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 43:
#line 176 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = ctx_.ident_create(msv((yystack_[3].value.val_str)), (yystack_[1].value.val_int)); }
#line 871 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 44:
#line 180 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(AstType::If); }
#line 877 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 48:
#line 190 "src/lang.y" // lalr1.cc:859
    { std::cerr << "NUMBER_DISCRETE not impl\n"; }
#line 883 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 49:
#line 191 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(utils::parse_integer(msv((yystack_[0].value.val_str)))); }
#line 889 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 50:
#line 192 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn((yystack_[0].value.val_float)); }
#line 895 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 56:
#line 206 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 901 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 57:
#line 207 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = mkn(AstType::Uminus, (yystack_[0].value.val_node)); }
#line 907 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 58:
#line 208 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 913 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 59:
#line 209 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = ctx_.create_binary_op(AstType::Add, (yystack_[2].value.val_node), (yystack_[0].value.val_node)); }
#line 919 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 60:
#line 210 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = ctx_.create_binary_op(AstType::Add, (yystack_[2].value.val_node), mkn(AstType::Uminus, (yystack_[0].value.val_node))); }
#line 925 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 61:
#line 211 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = ctx_.create_binary_op(AstType::Mul, (yystack_[2].value.val_node), (yystack_[0].value.val_node)); }
#line 931 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 62:
#line 212 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = ctx_.create_binary_op(AstType::Div, (yystack_[2].value.val_node), (yystack_[0].value.val_node)); }
#line 937 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 63:
#line 213 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 943 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;

  case 64:
#line 214 "src/lang.y" // lalr1.cc:859
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 949 "src/generated/lang.tab.cc" // lalr1.cc:859
    break;


#line 953 "src/generated/lang.tab.cc" // lalr1.cc:859
            default:
              break;
            }
        }
      catch (const syntax_error& yyexc)
        {
          error (yyexc);
          YYERROR;
        }
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, yylhs);
    }
    goto yynewstate;

  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        error (yyla.location, yysyntax_error_ (yystack_[0].state, yyla));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.type_get () == yyeof_)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:

    /* Pacify compilers like GCC when the user code never invokes
       YYERROR and the label yyerrorlab therefore never appears in user
       code.  */
    if (false)
      goto yyerrorlab;
    yyerror_range[1].location = yystack_[yylen - 1].location;
    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    goto yyerrlab1;

  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    {
      stack_symbol_type error_token;
      for (;;)
        {
          yyn = yypact_[yystack_[0].state];
          if (!yy_pact_value_is_default_ (yyn))
            {
              yyn += yyterror_;
              if (0 <= yyn && yyn <= yylast_ && yycheck_[yyn] == yyterror_)
                {
                  yyn = yytable_[yyn];
                  if (0 < yyn)
                    break;
                }
            }

          // Pop the current state because it cannot handle the error token.
          if (yystack_.size () == 1)
            YYABORT;

          yyerror_range[1].location = yystack_[0].location;
          yy_destroy_ ("Error: popping", yystack_[0]);
          yypop_ ();
          YY_STACK_PRINT ();
        }

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = yyn;
      yypush_ ("Shifting", error_token);
    }
    goto yynewstate;

    // Accept.
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;

    // Abort.
  yyabortlab:
    yyresult = 1;
    goto yyreturn;

  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack"
                 << std::endl;
        // Do not try to display the values of the reclaimed symbols,
        // as their printer might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type yystate, const symbol_type& yyla) const
  {
    // Number of reported tokens (one for the "unexpected", one per
    // "expected").
    size_t yycount = 0;
    // Its maximum.
    enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
    // Arguments of yyformat.
    char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];

    /* There are many possibilities here to consider:
       - If this state is a consistent state with a default action, then
         the only way this function was invoked is if the default action
         is an error action.  In that case, don't check for expected
         tokens because there are none.
       - The only way there can be no lookahead present (in yyla) is
         if this state is a consistent state with a default action.
         Thus, detecting the absence of a lookahead is sufficient to
         determine that there is no unexpected or expected token to
         report.  In that case, just report a simple "syntax error".
       - Don't assume there isn't a lookahead just because this state is
         a consistent state with a default action.  There might have
         been a previous inconsistent state, consistent state with a
         non-default action, or user semantic action that manipulated
         yyla.  (However, yyla is currently not documented for users.)
       - Of course, the expected token list depends on states to have
         correct lookahead information, and it depends on the parser not
         to perform extra reductions after fetching a lookahead from the
         scanner and before detecting a syntax error.  Thus, state
         merging (from LALR or IELR) and default reductions corrupt the
         expected token list.  However, the list is correct for
         canonical LR with one exception: it will still contain any
         token that will not be accepted due to an error action in a
         later state.
    */
    if (!yyla.empty ())
      {
        int yytoken = yyla.type_get ();
        yyarg[yycount++] = yytname_[yytoken];
        int yyn = yypact_[yystate];
        if (!yy_pact_value_is_default_ (yyn))
          {
            /* Start YYX at -YYN if negative to avoid negative indexes in
               YYCHECK.  In other words, skip the first -YYN actions for
               this state because they are default actions.  */
            int yyxbegin = yyn < 0 ? -yyn : 0;
            // Stay within bounds of both yycheck and yytname.
            int yychecklim = yylast_ - yyn + 1;
            int yyxend = yychecklim < yyntokens_ ? yychecklim : yyntokens_;
            for (int yyx = yyxbegin; yyx < yyxend; ++yyx)
              if (yycheck_[yyx + yyn] == yyx && yyx != yyterror_
                  && !yy_table_value_is_error_ (yytable_[yyx + yyn]))
                {
                  if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                    {
                      yycount = 1;
                      break;
                    }
                  else
                    yyarg[yycount++] = yytname_[yyx];
                }
          }
      }

    char const* yyformat = YY_NULLPTR;
    switch (yycount)
      {
#define YYCASE_(N, S)                         \
        case N:                               \
          yyformat = S;                       \
        break
        YYCASE_(0, YY_("syntax error"));
        YYCASE_(1, YY_("syntax error, unexpected %s"));
        YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
        YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
        YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
        YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
      }

    std::string yyres;
    // Argument number.
    size_t yyi = 0;
    for (char const* yyp = yyformat; *yyp; ++yyp)
      if (yyp[0] == '%' && yyp[1] == 's' && yyi < yycount)
        {
          yyres += yytnamerr_ (yyarg[yyi++]);
          ++yyp;
        }
      else
        yyres += *yyp;
    return yyres;
  }


  const signed char parser::yypact_ninf_ = -18;

  const signed char parser::yytable_ninf_ = -1;

  const signed char
  parser::yypact_[] =
  {
       7,    11,    16,    22,    96,   -18,   -18,     4,     4,   -18,
     -18,    12,   -18,     4,   -18,   -18,    66,   -18,    35,   -18,
     -18,   -18,   -18,     3,   -18,   -18,   -18,   -18,   -18,   -18,
      -5,   -18,   -18,    29,   -18,   -18,    72,   -18,   -18,    86,
     -18,   -18,   -18,    -5,    -5,    -5,   -18,   -18,    -7,   -10,
      -5,   -18,    31,   -18,   -18,   -18,    73,    73,    33,   -18,
     -18,    -5,    -5,    -5,    -5,    -5,    -5,    -5,     1,    63,
     -18,    38,    70,    73,    73,   -18,   -18,    70,    70,   -18,
     -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,   -18,
       2,    -5,   -18,   -18,   -18,   -18,   -18,   -18,    62,    60,
     -18,   -18,    38,   -18
  };

  const unsigned char
  parser::yydefact_[] =
  {
       0,     0,     0,     0,     0,     2,    32,     0,     0,     1,
       3,    13,    35,    13,    10,    12,     0,    39,     0,    33,
      34,    37,    11,     0,     9,     6,    41,     7,    21,    22,
       0,     4,    14,     0,    15,    16,     0,     8,     5,     0,
      48,    49,    50,     0,     0,     0,    64,    63,     0,    51,
       0,    36,     0,    38,    40,    42,    58,    57,     0,    13,
      55,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      56,     0,    53,    59,    60,    61,    62,    52,    54,    17,
      18,    23,    26,    25,    28,    27,    30,    29,    31,    24,
       0,     0,    13,    47,    44,    19,    20,    43,     0,     0,
      13,    46,     0,    45
  };

  const signed char
  parser::yypgoto_[] =
  {
     -18,   -18,    74,   -18,   -18,   -18,    78,    40,   -13,   -18,
     -18,   -18,   -17,   -18,   -18,   -18,    89,   -18,   -18,   -18,
     -18,   -18,    69,   -18,     0,   -18,    20,    -1
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     4,     5,    31,    38,    25,    13,    14,    18,    32,
      81,    97,    46,    34,    90,    11,    15,    21,    36,    20,
      26,    39,    53,    35,    94,    47,    48,    49
  };

  const unsigned char
  parser::yytable_[] =
  {
      23,    33,    79,    95,    60,    61,    33,    28,    29,    37,
       1,    59,     2,    12,     3,    28,    29,    40,    41,    42,
      30,    12,    17,     6,    62,    63,    64,    65,     7,    43,
      44,    66,    67,    45,     8,    62,    63,    64,    65,    27,
      80,    96,    56,    57,    58,    50,    71,    28,    29,    68,
      28,    29,    30,    22,    33,    30,    22,    91,    92,    93,
      72,    73,    74,    75,    76,    77,    78,    62,    63,    64,
      65,    69,    28,    29,    24,    12,    70,    30,    10,    99,
     100,   101,    33,    51,    52,    33,    16,   102,    82,    83,
      84,    85,    86,    87,    88,    89,     9,    54,    52,     1,
      19,     2,   103,     3,    62,    63,    64,    65,    55,    64,
      65,    98
  };

  const unsigned char
  parser::yycheck_[] =
  {
      13,    18,     1,     1,    14,    15,    23,    12,    13,     6,
       3,    18,     5,     9,     7,    12,    13,    22,    23,    24,
      17,     9,    10,    12,    34,    35,    36,    37,    12,    34,
      35,    41,    42,    38,    12,    34,    35,    36,    37,     4,
      39,    39,    43,    44,    45,    16,    59,    12,    13,    50,
      12,    13,    17,    13,    71,    17,    16,    19,    20,    21,
      61,    62,    63,    64,    65,    66,    67,    34,    35,    36,
      37,    40,    12,    13,     8,     9,    43,    17,     4,    92,
      18,    21,    99,    11,    12,   102,     8,   100,    25,    26,
      27,    28,    29,    30,    31,    32,     0,    11,    12,     3,
      11,     5,   102,     7,    34,    35,    36,    37,    39,    36,
      37,    91
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,     3,     5,     7,    45,    46,    12,    12,    12,     0,
      46,    59,     9,    50,    51,    60,    50,    10,    52,    60,
      63,    61,    51,    52,     8,    49,    64,     4,    12,    13,
      17,    47,    53,    56,    57,    67,    62,     6,    48,    65,
      22,    23,    24,    34,    35,    38,    56,    69,    70,    71,
      16,    11,    12,    66,    11,    66,    71,    71,    71,    18,
      14,    15,    34,    35,    36,    37,    41,    42,    71,    40,
      43,    52,    71,    71,    71,    71,    71,    71,    71,     1,
      39,    54,    25,    26,    27,    28,    29,    30,    31,    32,
      58,    19,    20,    21,    68,     1,    39,    55,    70,    52,
      18,    21,    52,    68
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    44,    45,    45,    46,    46,    46,    47,    48,    49,
      50,    50,    51,    52,    52,    53,    53,    54,    54,    55,
      55,    56,    56,    57,    58,    58,    58,    58,    58,    58,
      58,    58,    59,    59,    59,    61,    60,    62,    62,    64,
      63,    65,    65,    66,    67,    68,    68,    68,    69,    69,
      69,    70,    70,    70,    70,    70,    71,    71,    71,    71,
      71,    71,    71,    71,    71
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     1,     2,     5,     5,     4,     1,     1,     1,
       1,     2,     1,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     2,     2,     0,     4,     0,     2,     0,
       4,     0,     2,     4,     5,     5,     3,     1,     1,     1,
       1,     1,     3,     3,     3,     2,     3,     2,     2,     3,
       3,     3,     3,     1,     1
  };



  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "$end", "error", "$undefined", "PROGRAM", "END_PROGRAM", "FUNCTION",
  "END_FUNCTION", "FUNCTION_BLOCK", "END_FUNCTION_BLOCK", "VAR",
  "VAR_GLOBAL", "END_VAR", "IDENTIFIER", "EXTERNAL_IDENTIFIER", "GE", "LE",
  "ASSIGNMENT", "IF", "THEN", "ELSIF", "ELSE", "END_IF", "NUMBER_DISCRETE",
  "NUMBER_INTEGER", "NUMBER_FLOAT", "NPT_I8", "NPT_U8", "NPT_I16",
  "NPT_U16", "NPT_I32", "NPT_U32", "NPT_F32", "NPT_BOOL", "','", "'+'",
  "'-'", "'*'", "'/'", "'('", "';'", "':'", "'<'", "'>'", "')'", "$accept",
  "module", "module0", "end_program", "end_function", "end_function_block",
  "function", "function0", "stmt_list", "stmt", "semicolon", "semicolon1",
  "ident", "assignment", "var_type", "prog_decl", "var_decl_local", "$@1",
  "var_decl_seq", "var_decl_global", "$@2", "var_decl_seq_global",
  "var_decl0", "if_stmt", "else_clause", "number", "logical_exp", "exp", YY_NULLPTR
  };

#if YYDEBUG
  const unsigned char
  parser::yyrline_[] =
  {
       0,    79,    79,    80,    84,    88,    89,    93,    97,   101,
     105,   106,   110,   114,   115,   119,   120,   124,   124,   128,
     128,   132,   133,   137,   141,   142,   143,   144,   145,   146,
     147,   148,   152,   153,   154,   158,   158,   162,   163,   167,
     167,   171,   172,   176,   180,   184,   185,   186,   190,   191,
     192,   197,   198,   199,   200,   201,   206,   207,   208,   209,
     210,   211,   212,   213,   214
  };

  // Print the state stack on the debug stream.
  void
  parser::yystack_print_ ()
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << i->state;
    *yycdebug_ << std::endl;
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):" << std::endl;
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  // Symbol number corresponding to token number t.
  inline
  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    static
    const token_number_type
    translate_table[] =
    {
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      38,    43,    36,    34,    33,    35,     2,    37,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    40,    39,
      41,     2,    42,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32
    };
    const unsigned int user_token_number_max_ = 287;
    const token_number_type undef_token_ = 2;

    if (static_cast<int>(t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned int> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} // yy
#line 1449 "src/generated/lang.tab.cc" // lalr1.cc:1167
#line 217 "src/lang.y" // lalr1.cc:1168


namespace yy {
  // Report an error to the user.
  void parser::error (const location_type& loc, const std::string& msg) {
    std::cerr << '[' << loc << "] " << msg << '\n';
  }
}

