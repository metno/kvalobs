#include <iterator>
#include <algorithm>
#include <string>
#include <vector>
#include <iostream>

/*
 * Templated round function
 *
 * @author: Danushka "silvermace" Abeysuriya
 * @template param: T, data type and return type
 * @template param: Ndp, the number of decimal places to round to
 * @param: val, the value to be rounded
 * @returns: rounded val to Ndp decimal places of type T
 */

///Template to allow the rounding of data values (credits: Danushka Abeysuriya)

template< typename T, const int Ndp >
T round( const T val )
{
	T raised  = pow( 10, (double)( abs(Ndp) ) );
	T temp 	 = val * raised;
	T rounded = static_cast<T>( floor( temp ) );

	if( temp - rounded >= (T)0.5 ) {
		rounded = static_cast<T>( ceil( temp ) );
	}

	return static_cast<T>(rounded / raised);
}
