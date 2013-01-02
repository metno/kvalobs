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
   * Listen kan faktisk vaere separert med et hvilket som helst tegn også
   * en sekvens av tegn (streng), men default brukes et komma. Det finnes 
   * forskjellige konstruktorer som kan brukes for aa angi et annet separator 
   * tegn.
   *
   * Hvis et element har en verdi hvor separator tegnet ingår må verdien
   * omsluttes med ". 
   *
   *  Eks.
   *     Separator tegn er et komma, (,).
   *
   *     Verdi: 3,14 dette må settes in som "3,14".
   */
class CommaString
{
  struct Elem{
    std::string data;
    bool        isString;

    Elem(const std::string &d, bool b=false):data(d), isString(b){}
    Elem(const Elem &e):data(e.data), isString(e.isString){}
    Elem():isString(false){}
    
    Elem& operator=(const Elem &rhs){
      if(this!=&rhs){
	data=rhs.data;
	isString=rhs.isString;
      }

      return *this;
    }

    void erase(){ data.erase(); isString=false;}
  };
    
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
     * Indekser er i område [0,size()>. Funksjonen returnerer false
     * hvis val ikke kan settes inn i kommalisten. Insert trimmer val 
     * for space (både forran og bak) før den settes inn i kommastrengen. Dette
     * gjøres uten å endre val.
     */
    bool  insert(unsigned int index, const char *val);
    bool  insert(unsigned int index, const std::string &val);
    
    /**
     * \exception std::range_error når index er >= antall element 
     *  i listen.
     */
    std::string& operator[](const int index);
    const std::string& operator[](const int index)const;

    /**
     * returns false når index >= anatall element i listen.
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
     * vil strengen bli trunkert. Bufferet må være stort nok til å
     * holde '\0' termineringen. Dvs. størrelsen må væreminst  length()+1.
     *
     * @param str en peker til bufferet vi skal kopierer den komma
     *            separerte strengen til.
     * @param size størrelsen på str.
     * @ret   true  dersom helestrengen lot seg kopiere over i str.
     *        false dersom bufferet ikke er stort nok til å holde hele
     *              strengen + '\0'.
     */
    bool copy(char *str, int size)const;
    void copy(std::string &str)const;

    friend std::ostream& operator<<(std::ostream &, const CommaString &);

    std::ostream& print(std::ostream &ostr)const;
};


std::ostream& operator<<(std::ostream &, const CommaString &);

/** @} */ 
}
#endif
