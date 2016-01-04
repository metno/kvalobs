#ifndef __ArgvClass_h__
#define __ArgvClass_h__

#include <string>
#include <list>
#include "miconfparser/miconfparser.h"

namespace miutil {

class ArgvClass {
  ArgvClass(const ArgvClass &);
  ArgvClass& operator=(const ArgvClass &);

  void split(std::list<std::string> &strList, const std::string &buf);

  char **argv;
  unsigned int n;

 public:

  /**
   * Constructor som tar en ConfSection og lager en argv liste på
   * formen. Kun enkle keys er brukt, keys i subsections er ikke brukt.
   *      argv[0]="key1=val1"
   *      argv[1]="key2=val2"
   *      argv[n-1]="key=val"
   *      argv[n]=NULL
   */
  //ArgvClass(const conf::ConfSection &collection);
  /**
   * Konstruktor som tar en streng med keys, separert med mellomrom
   * og oppretter en liste. Eks.
   *
   * str="cmd arg1 arg2"
   * blir til listen.
   *
   *   argv[0]="cmd"
   *   argv[1]="arg1"
   *   argv[2]="arg2"
   *   argv[3]=NULL
   */

  ArgvClass(const std::string &str);
  ~ArgvClass();

  operator bool() {
    return argv != 0;
  }

  /**
   * getArgv returnerer en peker til første element element i listen.
   * Den returnerte pekeren eies av klassen og må behandles som
   * en konstant. Klassen må eksistere så lenge vi refererer pekeren.
   * Det allokerte området blir dealokert når klassen går ut av scope.
   * Enhver referering av pekeren vil mest sansynlig medføre core dump,
   * etter at klassen er gått ut av scope.
   */
  char * const * getArgv();

  /**
   * getArgn returnerer antall element i listen.
   */
  unsigned int getArgn();
};

}
#endif
