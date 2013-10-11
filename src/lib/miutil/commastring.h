/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: commastring.h,v 1.1.2.2 2007/09/27 09:02:32 paule Exp $                                                       

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
#ifndef __commastring_h__
#define __commastring_h__

#include <iosfwd>
#include <vector>
#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace miutil{
  
  /**
   * \addtogroup pu_miutil
   * @{
   */
  /**
   * \brief CommaString er en hjelpeklasse som administrerer en komma 
   * separert liste med verdier. 
   *
   * Verdiene har index fra [0, maxverdier>.
   *
   * Listen kan faktisk vaere separert med et hvilket som helst tegn ogs�
   * en sekvens av tegn (streng), men default brukes et komma. Det finnes 
   * forskjellige konstruktorer som kan brukes for aa angi et annet separator 
   * tegn.
   *
   * Hvis et element har en verdi hvor separator tegnet ing�r m� verdien
   * omsluttes med ". 
   *
   *  Eks.
   *     Separator tegn er et komma, (,).
   *
   *     Verdi: 3,14 dette m� settes in som "3,14".
   */
class CommaString
{
public:
  struct Elem : virtual public std::string {
    //std::string data_;
    bool        isString;

    Elem(const std::string &d, bool b=false): std::string(d), isString(b){
    	if( d.size() >= 2 && d[0]=='\"' && d[d.size()-1]== '\"') {
    		assign( d, 1, d.size() - 2 );
    		isString = true;
    	}
    }
    Elem(const Elem &e):std::string( e ), isString(e.isString){}
    Elem():isString(false){}
    

    Elem& operator=(const Elem &rhs){
      if(this!=&rhs){
    	  assign( rhs );
    	  isString=rhs.isString;
      }

      return *this;
    }

	template <typename T> T as( const T &defaultValue ) const
	{
		if( empty() )
			return defaultValue;

		return boost::lexical_cast<T>( *this );
	}

	template <typename T> T as() const
	{
		return boost::lexical_cast<T>( *this );
	}

    void erase(){ erase(); isString=false;}
  };
private:
  std::vector<Elem> data;
  std::string       separator;
  
  int   count(const std::string &str, char sep);
  int   count(const std::string &str, const std::string &sep);

 public:

    CommaString()
	:separator(",")
	{
	}

    CommaString(const CommaString &c)
	:data(c.data), separator(c.separator)
	{
	}



    /**
     * \brief Konstruktor som allokerer en kommaseparert liste med antElement.
     *
     * @param antElement Antall element listen skal initieres med.
     */
    CommaString(unsigned int antElement, char sep=',');

    /**
     * \brief Konstruktor som allokerer en kommaseparert liste med antElement.
     *
     * 
     *
     * @param antElement Antall element listen skal initieres med.
     */
    CommaString(unsigned int antElement, const std::string &sep=",");

    
    /**
     * \brief Konstruktor som initialserer CommaString med en allerede 
     * kommaseparert string. 
     *
     * @param commaList Kommaseparert streng.
     */
    CommaString(const char *commaList, char sep=',');

    /**
     * \brief Konstruktor som initialserer CommaString med en allerede 
     * kommaseparert string. 
     *
     * @param commaList Kommaseparert streng.
     */
    CommaString(const char *commaList, const std::string &sep);

    /**
     * \brief Konstruktor som initialiserer CommaString med en allerede 
     * kommaseparert string. 
     *
     * @param commaList Kommaseparert streng.
     */
    CommaString(const std::string &commaList, char sep=',');


    /**
     * \brief Konstruktor som initialiserer CommaString med en allerede 
     * kommaseparert string. 
     *
     * @param commaList Kommaseparert streng.
     */
    CommaString(const std::string &commaList, const std::string &sep);


    virtual ~CommaString()
	{
	}


    /**
     * \brief Reinitialize this CommaString with the commastring represented
     * with str. 
     *
     * The elements in str is separeted with the character sep.
     *
     * \param str A string with values separatede with character \em sep.
     *        The default value of \em sep is ','.
     * \param sep Use this as the separator to break up the \em str.
     * \return true if successfull false otherwise.
     */
    void init(const std::string &str, char sep=',');


    /**
     * \brief Reinitialize this CommaString with the commastring represented
     * with str. 
     *
     * The elements in str is separeted with the a sequens of character
     * given with the string sep.
     *
     * \param str A string with values separatede with character \em sep.
     *        The default value of \em sep is ','.
     * \param sep Use this as the separator to break up the \em str.
     * \return true if successfull false otherwise.
     */
    void init(const std::string &str, const std::string &sep);


    CommaString& operator=(const CommaString &s);

    /**
     * \brief Remove all dataelemens in the list, ie all element
     * get empty values.
     */
    void erase();


    /**
     * \brief Set the value at \em pos to the empty value.
     *
     * \param pos The index to the value to be erased.
     * \return  false if pos is not in the range [0,size()>, and true 
     *        otherwise.
     */
    bool erase(unsigned int pos);

    /**
     * \brief Sett in \em val at index.
     * 
     * Indekser er i omr�de [0,size()>. Funksjonen returnerer false
     * hvis val ikke kan settes inn i kommalisten. Insert trimmer val 
     * for space (b�de forran og bak) f�r den settes inn i kommastrengen. Dette
     * gj�res uten � endre val.
     */
    bool  insert(unsigned int index, const char *val);
    bool  insert(unsigned int index, const std::string &val);
    
    /**
     * \exception std::range_error n�r index er >= antall element 
     *  i listen.
     */
    Elem& operator[](const int index);
    const Elem& operator[](const int index)const;

    /**
     * returns false n�r index >= anatall element i listen.
     */
    bool  get(unsigned int index, std::string &)const;
    std::string  getSeparator()const{ return separator;}

    /**
     * size returnerer anttal element som det er avsatt plass til i
     * commalisten.
     */
    int   size()const{ return data.size();}
    
    /**
     * length returnerer antall byte som kommalisten
     * opptar, inkludert separator tegnet.
     */
    int   length()const;

    /**
     * copy kopirerer kommalisten over i en (char*) streng. Hvis
     * strengen vi skal kopiere data til er mindre enn kommastrengen
     * vil strengen bli trunkert. Bufferet m� v�re stort nok til �
     * holde '\0' termineringen. Dvs. st�rrelsen m� v�reminst  length()+1.
     *
     * @param str en peker til bufferet vi skal kopierer den komma
     *            separerte strengen til.
     * @param size st�rrelsen p� str.
     * @ret   true  dersom helestrengen lot seg kopiere over i str.
     *        false dersom bufferet ikke er stort nok til � holde hele
     *              strengen + '\0'.
     */
    bool copy(char *str, int size)const;
    void copy(std::string &str)const;

    friend std::ostream& operator<<(std::ostream &, const CommaString &);

    std::ostream& print(std::ostream &ostr)const;
};


std::ostream& operator<<(std::ostream &, const CommaString &);
std::ostream& operator<<(std::ostream &, const  CommaString::Elem &elem);

/** @} */ 
}
#endif
