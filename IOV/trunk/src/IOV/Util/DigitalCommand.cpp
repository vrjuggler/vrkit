// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>
#include <algorithm>
#include <iterator>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit.hpp>
#include <boost/spirit/tree/ast.hpp>

#include <IOV/Status.h>
#include <IOV/WandInterface.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Util/DigitalCommand.h>


namespace spirit = boost::spirit;

namespace
{

struct CommandGrammar : public spirit::grammar<CommandGrammar>
{
   static const int expressionID      = 1;
   static const int onActionID        = 2;
   static const int offActionID       = 3;
   static const int toggleOnActionID  = 4;
   static const int toggleOffActionID = 5;
   static const int negationID        = 6;
   static const int andExpressionID   = 7;
   static const int xorExpressionID   = 8;
   static const int orExpressionID    = 9;

   template<typename ScannerT>
   class definition
   {
   public:
      definition(const CommandGrammar& /* self */)
      {
         using namespace boost::spirit;

         // Terminals.
         // NOTE: The use of lexeme_d here is to try to enforce some
         // readability for end users. For example, "1 ^ ^ 3 -" seems a little
         // less meaningful (to me) than "1^ ^ 3-".
         on_action
            = lexeme_d[leaf_node_d[uint_p] >> root_node_d[ch_p('+')]]
            ;
         off_action
            = lexeme_d[leaf_node_d[uint_p] >> root_node_d[ch_p('-')]]
            ;
         toggle_on_action
            = lexeme_d[leaf_node_d[uint_p] >> root_node_d[ch_p('^')]]
            ;
         toggle_off_action
            = lexeme_d[leaf_node_d[uint_p] >> root_node_d[ch_p('v')]]
            ;

         action
            = on_action
            | off_action
            | toggle_on_action
            | toggle_off_action
            ;

         negation
            = ch_p('!')
            ;

         action_expr
            = ! root_node_d[negation] >> action
            | ! root_node_d[negation] >> inner_node_d['(' >> expression >> ')']
            ;

         // Non-terminals: conditional expressions.
         and_expression
            = action_expr >> *(root_node_d[ch_p('&')] >> action_expr)
            ;
         exclusive_or_expr
            = and_expression >> *(root_node_d[ch_p('^')] >> and_expression)
            ;
         inclusive_or_expr
            = exclusive_or_expr >> *(root_node_d[ch_p('|')] >> exclusive_or_expr)
            ;

         expression
            = inclusive_or_expr
            ;
      }

      const spirit::rule<ScannerT, spirit::parser_context<>, spirit::parser_tag<expressionID> >&
      start() const
      {
         return expression;
      }

   private:
      spirit::rule<
         ScannerT, spirit::parser_context<>, spirit::parser_tag<expressionID>
      > expression;

      spirit::rule<
         ScannerT, spirit::parser_context<>, spirit::parser_tag<onActionID>
      > on_action;
      spirit::rule<
         ScannerT, spirit::parser_context<>, spirit::parser_tag<offActionID>
      > off_action;
      spirit::rule<
         ScannerT, spirit::parser_context<>,
         spirit::parser_tag<toggleOnActionID>
      > toggle_on_action;
      spirit::rule<
         ScannerT, spirit::parser_context<>,
         spirit::parser_tag<toggleOffActionID>
      > toggle_off_action;

      spirit::rule<
         ScannerT, spirit::parser_context<>, spirit::parser_tag<negationID>
      > negation;

      spirit::rule<ScannerT> action;
      spirit::rule<ScannerT> action_expr;

      spirit::rule<
         ScannerT, spirit::parser_context<>,
         spirit::parser_tag<andExpressionID>
      > and_expression;
      spirit::rule<
         ScannerT, spirit::parser_context<>,
         spirit::parser_tag<xorExpressionID>
      > exclusive_or_expr;
      spirit::rule<
         ScannerT, spirit::parser_context<>,
         spirit::parser_tag<orExpressionID>
      > inclusive_or_expr;
   };
};

class ButtonQuery
{
public:
   ButtonQuery(gadget::DigitalInterface& digitalIf,
               const gadget::Digital::State testState)
      : mButtonIf(digitalIf)
      , mTestState(testState)
   {
   }

   bool operator()() const
   {
      return mButtonIf->getData() == mTestState;
   }

private:
   gadget::DigitalInterface& mButtonIf;
   const gadget::Digital::State mTestState;
};

class and_
{
public:
   and_(boost::function<bool()> lhs, boost::function<bool()> rhs)
      : lhs(lhs)
      , rhs(rhs)
   {
   }

   bool operator()() const
   {
      return lhs() && rhs();
   }

private:
   boost::function<bool ()> lhs;
   boost::function<bool ()> rhs;
};

class xor_
{
public:
   xor_(boost::function<bool()> lhs, boost::function<bool()> rhs)
      : lhs(lhs)
      , rhs(rhs)
   {
   }

   bool operator()() const
   {
      return (lhs || rhs) && ! (lhs && rhs);
   }

private:
   boost::function<bool ()> lhs;
   boost::function<bool ()> rhs;
};

class or_
{
public:
   or_(boost::function<bool()> lhs, boost::function<bool()> rhs)
      : lhs(lhs)
      , rhs(rhs)
   {
   }

   bool operator()() const
   {
      return lhs() || rhs();
   }

private:
   boost::function<bool ()> lhs;
   boost::function<bool ()> rhs;
};

class not_
{
public:
   not_(boost::function<bool ()> f)
      : f(f)
   {
   }

   bool operator()() const
   {
      return ! f();
   }

private:
   boost::function<bool ()> f;
};

class indent
{
public:
   indent(const unsigned int level)
      : mLevel(level)
   {
      /* Do nothing. */ ;
   }

   unsigned int level() const
   {
      return mLevel;
   }

private:
   const unsigned int mLevel;
};

std::ostream& operator<<(std::ostream& out, const indent& i)
{
   for ( unsigned int j = 0; j < i.level(); ++j )
   {
      out << "   ";
   }

   return out;
}

typedef const char* iterator_t;
typedef spirit::tree_match<iterator_t> parse_tree_match_t;
typedef parse_tree_match_t::const_tree_iterator iter_t;

unsigned int buttonIndex(const iter_t& i)
{
   const std::string index(i->value.begin(), i->value.end());
   return boost::lexical_cast<unsigned int>(index);
}

void printTree(const iter_t& i, const unsigned int level = 0)
{
   std::string v(i->value.begin(), i->value.end());
/*
   std::cout << indent(level) << "i->value = " << v
             << " i->children.size() = " << i->children.size() << std::endl;
*/
   if ( i->value.id() == CommandGrammar::onActionID )
   {
      const unsigned int index = buttonIndex(i->children.begin());
      std::cout << indent(level) << "on action: " << index << std::endl;
   }
   else if ( i->value.id() == CommandGrammar::offActionID )
   {
      const unsigned int index = buttonIndex(i->children.begin());
      std::cout << indent(level) << "off action: " << index << std::endl;
   }
   else if ( i->value.id() == CommandGrammar::toggleOnActionID )
   {
      const unsigned int index = buttonIndex(i->children.begin());
      std::cout << indent(level) << "toggle on action: " << index
                << std::endl;
   }
   else if ( i->value.id() == CommandGrammar::toggleOffActionID )
   {
      const unsigned int index = buttonIndex(i->children.begin());
      std::cout << indent(level) << "toggle off action: " << index
                << std::endl;
   }
   else if ( i->value.id() == CommandGrammar::andExpressionID )
   {
      std::cout << indent(level) << "and expression" << std::endl;
      printTree(i->children.begin(), level + 1);
      printTree(i->children.begin() + 1, level + 1);
   }
   else if ( i->value.id() == CommandGrammar::xorExpressionID )
   {
      std::cout << indent(level) << "xor expression" << std::endl;
      printTree(i->children.begin(), level + 1);
      printTree(i->children.begin() + 1, level + 1);
   }
   else if ( i->value.id() == CommandGrammar::orExpressionID )
   {
      std::cout << indent(level) << "or expression" << std::endl;
      printTree(i->children.begin(), level + 1);
      printTree(i->children.begin() + 1, level + 1);
   }
   else if ( i->value.id() == CommandGrammar::negationID )
   {
      std::cout << indent(level) << "negation" << std::endl;
      printTree(i->children.begin(), level + 1);
   }
   else
   {
      std::cout << indent(level) << "Unknown tree node type" << std::endl;
   }
}

/**
 * Composes functions to create a composite function that performs the test
 * for the state of the digital command based on the parse tree.
 */
boost::function<bool ()> eval(const iter_t& i, inf::WandInterfacePtr wandIf)
{
   boost::function<bool ()> f;

   if ( i->value.id() == CommandGrammar::onActionID )
   {
      f = ButtonQuery(wandIf->getButton(buttonIndex(i->children.begin())),
                      gadget::Digital::ON);
   }
   else if ( i->value.id() == CommandGrammar::offActionID )
   {
      f = ButtonQuery(wandIf->getButton(buttonIndex(i->children.begin())),
                      gadget::Digital::OFF);
   }
   else if ( i->value.id() == CommandGrammar::toggleOnActionID )
   {
      f = ButtonQuery(wandIf->getButton(buttonIndex(i->children.begin())),
                      gadget::Digital::TOGGLE_ON);
   }
   else if ( i->value.id() == CommandGrammar::toggleOffActionID )
   {
      f = ButtonQuery(wandIf->getButton(buttonIndex(i->children.begin())),
                      gadget::Digital::TOGGLE_OFF);
   }
   else if ( i->value.id() == CommandGrammar::andExpressionID )
   {
      f = and_(eval(i->children.begin(), wandIf),
               eval(i->children.begin() + 1, wandIf));
   }
   else if ( i->value.id() == CommandGrammar::xorExpressionID )
   {
      f = xor_(eval(i->children.begin(), wandIf),
               eval(i->children.begin() + 1, wandIf));
   }
   else if ( i->value.id() == CommandGrammar::orExpressionID )
   {
      f = or_(eval(i->children.begin(), wandIf),
              eval(i->children.begin() + 1, wandIf));
   }
   else if ( i->value.id() == CommandGrammar::negationID )
   {
      f = not_(eval(i->children.begin(), wandIf));
   }
   else
   {
      vprASSERT(false && "Encountered unexpected tree node type");
      // XXX: It might be worthwhile to throw an exception here for the
      // non-debug case.
   }

   return f;
}

}

namespace inf
{

DigitalCommand::DigitalCommand()
{
   /* Do nothing. */ ;
}

// buttonString is passed by copy on purpose so that boost::trim() can be used
// on it.
void DigitalCommand::configure(std::string buttonString,
                               inf::WandInterfacePtr wandIf)
{
   // Clean up buttonString before we try to use it.
   boost::trim(buttonString);

   // Backwards compatibility stuff.
   if ( buttonString.find(",") != std::string::npos )
   {
      IOV_STATUS << "WARNING: Use of the comma-separated button configuration "
                 << "is deprecated." << std::endl;

      // Convert the comma-separated list into the new syntax.
      std::vector<std::string> btn_strings;
      boost::split(btn_strings, buttonString, boost::is_any_of(", "));
      std::ostringstream btn_stream;
      std::copy(btn_strings.begin(), btn_strings.end(),
                std::ostream_iterator<std::string>(btn_stream, "^ & "));

      buttonString = btn_stream.str();

      // Chop off the trailing "& " added by the ostream iterator.
      buttonString = buttonString.substr(0, buttonString.size() - 2);

      IOV_STATUS << "         Consider using " << buttonString << " instead."
                 << std::endl;
   }
   else if ( buttonString.length() == 1 )
   {
      IOV_STATUS << "WARNING: Use of the comma-separated button configuration "
                 << "is deprecated." << std::endl;
      buttonString = buttonString + "+";
      IOV_STATUS << "         Consider using " << buttonString << " instead."
                 << std::endl;
   }
   else if ( std::string("-1") == buttonString )
   {
      IOV_STATUS << "WARNING: Use of '-1' for the button configuration is "
                 << "deprecated.\n"
                 << "         Consider using an empty string instead."
                 << std::endl;
      mTestFunc = &DigitalCommand::dummyTest;
      return;
   }
   else if ( buttonString.empty() )
   {
      mTestFunc = &DigitalCommand::dummyTest;
      return;
   }

   CommandGrammar g;
   spirit::tree_parse_info<> info = spirit::ast_parse(buttonString.c_str(),
                                                      g, spirit::space_p);

   if ( info.full )
   {
      mConfigString = buttonString;
      mTestFunc     = eval(info.trees.begin(), wandIf);
   }
   else
   {
      throw inf::Exception("Parse error", IOV_LOCATION);
   }
}

bool DigitalCommand::operator==(const DigitalCommand& rhs) const
{
   // XXX: This is not quite right. Two boolean expressions can be equivalent
   // without being identical. A more proper implementation is needed.
   return mConfigString == rhs.mConfigString;
}

}
