/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: confparser.h,v 1.1.2.2 2007/09/27 09:02:25 paule Exp $                                                       

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as 
  published by the Free Software Foundation; either version 2 
  of the License, or (at your option) any later version.
  
  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along 
  with KVALOBS; if not, write to the Free Software Foundation Inc., 
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef __miutil_conf_confparser_h__
#define __miutil_conf_confparser_h__

#include <stack>
#include <sstream>
#include <iosfwd>
#include <list>
#include <string>
#include <stdexcept>
#include <map>
#include <miconfparser/confsection.h>


/**
 * \defgroup miconfparser miconfparser, a parser of files in a specific format.
 * \ingroup miutil
 */
int yyConfParserlex( void );

namespace miutil{
  
  /**
   * \brief The namespace that the miconfparser is in.
   */
  namespace conf{
    /** 
     * \addtogroup miconfparser
     *
     * @{
     */ 
    
    /**
     * \brief The parser interface to the miconfparser library.
     */
    class ConfParser{
    public:
      //yyInput, newStream, deleteStream is logical private, but is defined as public
      //for easy integration with flex. Dont use it.
      int   yyInput(char *buf, int size_of_buf);
      bool  newStream(const std::string &file); 
      bool  deleteStream();
      void  cleanIstStack();
      
    private:
      static std::map<const ConfParser*, void*> pimpel;
      ConfParser(const ConfParser&);
      ConfParser& operator=(const ConfParser&);
      
      friend int ::yyConfParserlex ( void );
      
      
      typedef enum{ 
      		 MiTT_ID, MiTT_IGNORE_ID, MiTT_STRING, MiTT_INT,
		      MiTT_FLOAT, MiTT_ALIAS,
		      MiTT_EQUAL,  //= 
		      MiTT_OB,     //{
		      MiTT_CB,     //}
		      MiTT_OP,     //(
		      MiTT_CP,     //)
		      MiTT_NL,     //New Line
		      MiTT_COMMA   //,
			}TokenType;
      
      struct Token{ 
			TokenType tt;
			std::string val; 
			int         line;
			Token(TokenType tt_, const std::string &val_, int line_):
	  			tt(tt_), val(val_), line(line_) {}
			Token(const Token &t):tt(t.tt), val(t.val), line(t.line){}
			Token &operator=(const Token &t){
	  			if(this!=&t){
	    			tt=t.tt;
	    			val=t.val;
	    			line=t.line;
		    	}
	  			return *this;
			}
	
			std::string print()const;
      };
      
      struct TIstStack{
  	 		void         *state;
  	 		std::istream *ist;
  	 		int          lineno;
  	 		std::string  file;
  		};
      
      typedef std::list<ConfSection*>                   SectionStack;
      typedef std::list<ConfSection*>::iterator        ISectionStack;
      typedef std::list<ConfSection*>::const_iterator CISectionStack;
      typedef std::list<Token>                   TokenStack;
      typedef std::list<Token>::iterator        ITokenStack;
      typedef std::list<Token>::const_iterator CITokenStack;
      
      void intToken(const char *i);
      void floatToken(const char *d);
      void idToken(const char *id);
      void ignoreIdToken(const char *id);
      void aliasToken(const char *id);
      void charToken(char token);
      void stringToken(const char *s);
      void deleteStack();
      void deleteTokenStack();
      std::ostream& printTokenStack(std::ostream &ost)const;
      bool checkToken(const Token &t);
      bool colapseList();
      bool colapseKeyVal(const Token &t);
      bool colapseSection();
      

      //int  lineno_;
      //std::istream *ist_;
      std::stringstream errs_;
      bool         error_;
      SectionStack  stack_;
      bool         listContext_;
      TokenStack   tokenStack_;
      int          debugLevel_;
      std::stack<TIstStack*> istStack;
      TIstStack    *curIst;
      bool deleteIgnoredSections;

    public:
      /**
       * The defaul contructor. It does nothing.
       */
      ConfParser();
      ConfParser( bool allowMultipleSections );
      /**
       * A constructor that set the input that shall be parsed.
       * To start the parsing of the data the function parse() must
       * be called.
       *
       * \param ist parse the input stream ist.
       */
      ConfParser(std::istream &ist );

      ConfParser(std::istream &ist, bool allowMultipleSections );
      
      ~ConfParser();

      void        keepIgnoredSection();

      bool        allowMultipleSections()const;
      int         lineno()const { return (curIst?curIst->lineno:0);}
      std::string filename()const { return (curIst?curIst->file:"");}
      
      /**
       * \brief get the debug level
       *
       * that i used to log the the  progres of the
       * parsing of the input.
       * 
       * \return The loglevel.
       */
      int         debugLevel()const{ return debugLevel_;}

      /**
       * \brief set the debug level
       *
       * that i used to log the the  progres of the
       * parsing of the input.
       */
      void        debugLevel(int dl){ debugLevel_=dl;}

      /**
       * \brief parse a input stream
       *
       * that contains the configurations settings.
       * \param ist the input stream to be parsed.
       * \return a pointer to a ConfSections. 0 if there was
       *         an unrecoverable error while parsing.
       *         You must delete this pointer when it is not 
       *         needed anymore.
       */
      ConfSection *parse(std::istream &ist);
  
  
      /**
       * \brief parse a file.
       *
       * that contains the configurations settings.
       * \param the file to be parsed.
       * \return a pointer to a ConfSections. 
       *         The caller must delete this pointer when it is not 
       *         needed anymore.
       * 
       * \throws std::logic_error when the file dont exist, cant 
       * be opened or there is an error in the file. 
       */
      static ConfSection *parse( const std::string &filename );
      static ConfSection *parse( const std::string &filename,
                                 bool allowMultipleSections );
  
      /**
       * \brief parse a input stream
       *
       * that contains the configurations settings.
       * The inputstream to be parsed is that given in the constructor
       * ConfParser(std::istream &ist).
       * \return a pointer to a ConfSections. 0 if there was
       *         an unrecoverable error while parsing. You must delete 
       *         this pointer when it is not needed anymore.
       */
      ConfSection *parse();
      
      /**
       * \brief get the error message from the last call of parse.
       *
       * \return An error message from the last call of parse.
       */
      std::string getError()const{ return errs_.str();}
    };
    
    namespace priv{
      
      int  ConfParserLexer(ConfParser *parser);
    }
    
    /** @} */
  }
}
    


#endif
