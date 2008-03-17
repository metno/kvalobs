/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dso.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#ifndef __DSO_H__
#define __DSO_H__

#include <string>

namespace dnmi{
  namespace file{

    /**
     * \addtogroup fileutil
     *
     * @{
     */

      class DSOException : public std::exception{
	std::string reason;
      public:
	  DSOException(const std::string &reason_):reason(reason_){
	  }
	  ~DSOException()throw(){}
	  
	  const char *what()const throw(){ return reason.c_str();}
      };

    /**
     * \brief DSO (Dynamic Shared Object).
     *
     * Klassen brukes for å laste i shared objekt  dynamisk under 
     * programkjøring. Det kan bare være et shared objekt assossiert
     * med en instance av DSO over objektets levetid.
     *
     * \code
      Eks på bruk:
          Anta vi har en shared objekt file med navnet my_dso.so.
          Filen har definert en funksjone   void func(int)
     
       void (*f)(int);
     
       try{
         DSO dso("my_dso.so");
     
         //Laster inn func
         f=dso["func"];
     
         //Kan nå bruke func.
         f(19);
       }catch(const DSOException &e){
         std::cerr << "DSO: problemer <grunn: " << e << ">\n";
       }
       
       //Vi kan ikke bruke f her da det my_dso automatisk lukkes
       //når dso går ut av scope.
       \endcode
     */
          
    class DSO{
      DSO(const DSO &);
      DSO& operator=(const DSO &);

      void *handle;
      std::string dsofile;
      
    public:
      
      /**
       * Default konstruktor. Laster ingen fil.
       */
      DSO():handle(0)
	{
	}
      
      /**
       * \brief Konstruktor som laster dsoFile. 
       * 
       * Flagget resolveNow bestemmer om  alle eksterne referanser blir
       * løst nå eller ved første gangs bruk.
       *
       * \param dsoFile navnet på det dynamisk lastbare objektet vi ønsker
       *                å laste. Dette inkluderer også en path.
       * \param resolveNow Skal vi løse ut alle eksterne referanser med 
       *                   en gang eller vente.
       * \exception DSOException Hvis filen dsoFile ikke finnes eller
       *            det oppstår andre feil under lastingen av dsoFileen
       *            vil det bli gitt DSOException.
       * \see DLOPEN(3)
       */
      
      DSO(const std::string &dsoFile, bool resolveNow=true);
      
      /**
       * \brief Destruktoren lukker det dynamisk lastbare objektet. 
       *
       * Alle referanser til objektet vil være udefinert. Bruk av 
       * klasser/funksjoner som er lastet via denne instansen vil 
       * sannsynligvis føre til segmentation fault.
       */
      virtual ~DSO();
      
      /**
       * \brief Prøver å laste dsoFile. 
       *
       * Hvis det allerede er assosiert en dso fil til denne instansen
       * vil vi få DSOException.
       *
       * \param dsoFile dsoFilen vi ønsker å laste.
       * \param resolveNow Når skal vi løse ut eksterne referanser i dsoFilen.
       * \exception DSOException Hvis filen ikke finnes, det oppstår 
       *            problemer med å laste dsoFile 
       *            eller denne instansen representerer allerede en dsoFile.
       */
      void  load(const std::string &dsoFile, bool resolveNow=true);

      /**
       * \brief isValid kan kalles for å sjekke om instansen representerer en
       * gyldig dsoFile.
       *
       * \return true hvis instansen rtepresenterer en gyldig dsoFile. False
       *         ellers.
       */
      bool  isValid()const { return handle!=0;}

 
      /**
       * \brief returner navnet på DSO filen denne instansen representerer.
       */
      std::string getDsofile()const { return dsofile;}

      /**
       * \brief getLastError returnerer siste feil som har oppstått ved bruk
       * av dsoFile.
       *
       * \return en error streng.
       */
      std::string getLastError();

      /**
       * \brief Denne operatoren kan brukes for å finne symboler i dsoFilen.
       *
       * \param name Symbolet vi ønsker en referanse til.
       * \exception DSOException blir gitt dersom symbolet name
       *            ikke kan finnes i filen eller annen feil oppstår.
       *
       * \see DLSYM(3)
       */
      void* operator[](const std::string &name);
    };
    
    /** @} */
  };
};

#endif
