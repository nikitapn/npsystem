// A Bison parser, made by GNU Bison 3.3.2.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2019 Free Software Foundation, Inc.

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

// Undocumented macros, especially those whose name start with YY_,
// are private implementation details.  Do not rely on them.





#include "lang.tab.hh"


// Unqualified %code blocks.
#line 22 "src/lang.y" // lalr1.cc:435

	#include <iostream>
  #include <fstream>
  #include <string_view>
  
	extern int yylex(yy::parser::semantic_type* yylval, yy::parser::location_type* loc);

	using namespace npcompiler::ast;

	#define mkn(...) new AstNode(__VA_ARGS__)
	#define msv(x) std::string_view(x.ptr, x.len)

#line 58 "src/generated/lang.tab.cc" // lalr1.cc:435


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

// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
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
    while (false)
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
      *yycdebug_ << '\n';                       \
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
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)


namespace yy {
#line 153 "src/generated/lang.tab.cc" // lalr1.cc:510

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

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | Symbol types.  |
  `---------------*/

  // basic_symbol.
#if 201103L <= YY_CPLUSPLUS
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (basic_symbol&& that)
    : Base (std::move (that))
    , value (std::move (that.value))
    , location (std::move (that.location))
  {}
#endif

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (semantic_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}

  template <typename Base>
  bool
  parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return Base::type_get () == empty_symbol;
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_type.
  parser::by_type::by_type ()
    : type (empty_symbol)
  {}

#if 201103L <= YY_CPLUSPLUS
  parser::by_type::by_type (by_type&& that)
    : type (that.type)
  {
    that.clear ();
  }
#endif

  parser::by_type::by_type (const by_type& that)
    : type (that.type)
  {}

  parser::by_type::by_type (token_type t)
    : type (yytranslate_ (t))
  {}

  void
  parser::by_type::clear ()
  {
    type = empty_symbol;
  }

  void
  parser::by_type::move (by_type& that)
  {
    type = that.type;
    that.clear ();
  }

  int
  parser::by_type::type_get () const YY_NOEXCEPT
  {
    return type;
  }


  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_number_type
  parser::by_state::type_get () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return empty_symbol;
    else
      return yystos_[state];
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.type = empty_symbol;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
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
#if defined __GNUC__ && ! defined __clang__ && ! defined __ICC && __GNUC__ * 100 + __GNUC_MINOR__ <= 408
    // Avoid a (spurious) G++ 4.8 warning about "array subscript is
    // below array bounds".
    if (yysym.empty ())
      std::abort ();
#endif
    yyo << (yytype < yyntokens_ ? "token" : "nterm")
        << ' ' << yytname_[yytype] << " ("
        << yysym.location << ": ";
    YYUSE (yytype);
    yyo << ')';
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n)
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

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - yyntokens_] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - yyntokens_];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
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

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << yystack_[0].state << '\n';

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token: ";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.type = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
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
    yypush_ ("Shifting", yyn, YY_MOVE (yyla));
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
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2:
#line 68 "src/lang.y" // lalr1.cc:919
    { root_.push((yystack_[0].value.val_node)); }
#line 597 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 3:
#line 72 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::StmtList); }
#line 603 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 4:
#line 73 "src/lang.y" // lalr1.cc:919
    { (yystack_[1].value.val_node)->push((yystack_[0].value.val_node)); (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 609 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 5:
#line 77 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 615 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 6:
#line 78 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 621 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 7:
#line 79 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 627 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 8:
#line 80 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = nullptr; }
#line 633 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 9:
#line 84 "src/lang.y" // lalr1.cc:919
    { /*$$ = mkn(non_term, AstType::Assignment, $1, $3);*/ }
#line 639 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 10:
#line 88 "src/lang.y" // lalr1.cc:919
    { 
			auto ident = Identifier0{IdentType::Variable, msv((yystack_[0].value.val_str)), false};
			ctx_.ident_put(static_cast<Identifier&>(ident)); 
			(yylhs.value.val_node) =  mkn(std::move(ident)); 
		}
#line 649 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 11:
#line 93 "src/lang.y" // lalr1.cc:919
    { 
			auto ident = Identifier0{IdentType::Variable, msv((yystack_[2].value.val_str)), false};
			ctx_.ident_put(static_cast<Identifier&>(ident)); 
			(yylhs.value.val_node) = mkn(non_term, AstType::Assignment, mkn(std::move(ident)), (yystack_[0].value.val_node)); 
		}
#line 659 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 12:
#line 101 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::VarDeclSeq, (yystack_[0].value.val_node)); }
#line 665 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 13:
#line 102 "src/lang.y" // lalr1.cc:919
    { (yystack_[2].value.val_node)->push((yystack_[0].value.val_node)); (yylhs.value.val_node) = (yystack_[2].value.val_node); }
#line 671 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 14:
#line 106 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 677 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 15:
#line 110 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::If); }
#line 683 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 19:
#line 120 "src/lang.y" // lalr1.cc:919
    { std::cerr << "NUMBER_DISCRETE not impl\n"; }
#line 689 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 20:
#line 121 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn((yystack_[0].value.val_int)); }
#line 695 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 21:
#line 122 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn((yystack_[0].value.val_float)); }
#line 701 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 27:
#line 134 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = (yystack_[1].value.val_node); }
#line 707 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 28:
#line 135 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::Uminus, (yystack_[0].value.val_node)); }
#line 713 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 29:
#line 136 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 719 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 30:
#line 137 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::Add, (yystack_[2].value.val_node), (yystack_[0].value.val_node)); }
#line 725 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 31:
#line 138 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::Add, (yystack_[2].value.val_node), mkn(non_term, AstType::Uminus, (yystack_[0].value.val_node))); }
#line 731 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 32:
#line 139 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::Mul, (yystack_[2].value.val_node), (yystack_[0].value.val_node)); }
#line 737 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 33:
#line 140 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(non_term, AstType::Div, (yystack_[2].value.val_node), (yystack_[0].value.val_node)); }
#line 743 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 34:
#line 141 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = (yystack_[0].value.val_node); }
#line 749 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;

  case 35:
#line 142 "src/lang.y" // lalr1.cc:919
    { (yylhs.value.val_node) = mkn(ctx_.ident_get(msv((yystack_[0].value.val_str)))); }
#line 755 "src/generated/lang.tab.cc" // lalr1.cc:919
    break;


#line 759 "src/generated/lang.tab.cc" // lalr1.cc:919
            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;
      YY_STACK_PRINT ();

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
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
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

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
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
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
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

  // Generate an error message.
  std::string
  parser::yysyntax_error_ (state_type, const symbol_type&) const
  {
    return YY_("syntax error");
  }


  const signed char parser::yypact_ninf_ = -31;

  const signed char parser::yytable_ninf_ = -1;

  const signed char
  parser::yypact_[] =
  {
     -31,     6,    37,   -31,    -3,    39,     9,   -31,   -31,   -31,
     -31,   -31,    39,   -31,   -31,   -31,   -31,    39,    39,    39,
     -31,     7,     3,    13,   -31,   -12,    48,   -18,   -18,    44,
     -31,   -31,    39,    39,    39,    39,    39,    39,    39,    39,
       9,   -31,   -31,   -31,    36,    54,   -18,   -18,   -31,   -31,
      54,    54,    54,   -31,    39,   -31,   -31,   -31,    20,     2,
     -31,   -31,    36,   -31
  };

  const unsigned char
  parser::yydefact_[] =
  {
       3,     0,     2,     1,     0,     0,     0,     8,     4,     5,
       6,     7,     0,    35,    19,    20,    21,     0,     0,     0,
      34,     0,    22,    10,    12,     0,     0,    29,    28,     0,
       3,    26,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    14,     9,    27,     0,    24,    30,    31,    32,    33,
      23,    25,    11,    13,     0,     3,    18,    15,     0,     0,
       3,    17,     0,    16
  };

  const signed char
  parser::yypgoto_[] =
  {
     -31,   -31,   -30,   -31,   -31,   -11,   -31,   -31,   -31,   -21,
     -31,    -4,    -1
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     1,     2,     8,     9,    24,    25,    10,    11,    57,
      20,    21,    22
  };

  const unsigned char
  parser::yytable_[] =
  {
      44,    35,    36,    12,    40,     4,     3,    31,    32,     5,
      41,    26,    23,    61,     6,    30,    27,    28,    29,    39,
      33,    34,    35,    36,     7,    59,    37,    38,    60,    53,
      62,    45,    46,    47,    48,    49,    50,    51,    52,     4,
       4,    63,    13,     5,     5,    54,    55,    56,     6,     6,
      58,     0,    14,    15,    16,     0,    17,    18,     7,     7,
      19,    33,    34,    35,    36,    33,    34,    35,    36,    43,
      42,    33,    34,    35,    36
  };

  const signed char
  parser::yycheck_[] =
  {
      30,    19,    20,     6,    16,     3,     0,     4,     5,     7,
      22,    12,     3,    11,    12,     8,    17,    18,    19,     6,
      17,    18,    19,    20,    22,    55,    23,    24,     8,    40,
      60,    32,    33,    34,    35,    36,    37,    38,    39,     3,
       3,    62,     3,     7,     7,     9,    10,    11,    12,    12,
      54,    -1,    13,    14,    15,    -1,    17,    18,    22,    22,
      21,    17,    18,    19,    20,    17,    18,    19,    20,    25,
      22,    17,    18,    19,    20
  };

  const unsigned char
  parser::yystos_[] =
  {
       0,    27,    28,     0,     3,     7,    12,    22,    29,    30,
      33,    34,     6,     3,    13,    14,    15,    17,    18,    21,
      36,    37,    38,     3,    31,    32,    38,    38,    38,    38,
       8,     4,     5,    17,    18,    19,    20,    23,    24,     6,
      16,    22,    22,    25,    28,    38,    38,    38,    38,    38,
      38,    38,    38,    31,     9,    10,    11,    35,    37,    28,
       8,    11,    28,    35
  };

  const unsigned char
  parser::yyr1_[] =
  {
       0,    26,    27,    28,    28,    29,    29,    29,    29,    30,
      31,    31,    32,    32,    33,    34,    35,    35,    35,    36,
      36,    36,    37,    37,    37,    37,    37,    38,    38,    38,
      38,    38,    38,    38,    38,    38
  };

  const unsigned char
  parser::yyr2_[] =
  {
       0,     2,     1,     0,     2,     1,     1,     1,     1,     4,
       1,     3,     1,     3,     3,     5,     5,     3,     1,     1,
       1,     1,     1,     3,     3,     3,     2,     3,     2,     2,
       3,     3,     3,     3,     1,     1
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a yyntokens_, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "$end", "error", "$undefined", "IDENTIFIER", "GE", "LE", "ASSIGNMENT",
  "IF", "THEN", "ELSIF", "ELSE", "END_IF", "VAR", "NUMBER_DISCRETE",
  "NUMBER_INTEGER", "NUMBER_FLOAT", "','", "'+'", "'-'", "'*'", "'/'",
  "'('", "';'", "'<'", "'>'", "')'", "$accept", "script", "stmt_list",
  "stmt", "assignment", "var_decl0", "var_decl_seq", "var_decl", "if_stmt",
  "else_clause", "number", "logical_exp", "exp", YY_NULLPTR
  };


  const unsigned char
  parser::yyrline_[] =
  {
       0,    68,    68,    72,    73,    77,    78,    79,    80,    84,
      88,    93,   101,   102,   106,   110,   114,   115,   116,   120,
     121,   122,   126,   127,   128,   129,   130,   134,   135,   136,
     137,   138,   139,   140,   141,   142
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
    *yycdebug_ << '\n';
  }

  // Report on the debug stream that the rule \a yyrule is going to be reduced.
  void
  parser::yy_reduce_print_ (int yyrule)
  {
    unsigned yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  parser::token_number_type
  parser::yytranslate_ (int t)
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const token_number_type
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      21,    25,    19,    17,    16,    18,     2,    20,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    22,
      23,     2,    24,     2,     2,     2,     2,     2,     2,     2,
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
      15
    };
    const unsigned user_token_number_max_ = 270;
    const token_number_type undef_token_ = 2;

    if (static_cast<int> (t) <= yyeof_)
      return yyeof_;
    else if (static_cast<unsigned> (t) <= user_token_number_max_)
      return translate_table[t];
    else
      return undef_token_;
  }


} // yy
#line 1142 "src/generated/lang.tab.cc" // lalr1.cc:1242
#line 145 "src/lang.y" // lalr1.cc:1243


namespace yy {
  // Report an error to the user.
  void parser::error (const location_type& loc, const std::string& msg) {
    std::cerr << '[' << loc << "] " << msg << '\n';
  }
}

