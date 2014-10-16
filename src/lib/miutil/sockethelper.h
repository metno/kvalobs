#ifndef __SocketHelper_h__
#define __SocketHelper_h__

#include <string>

namespace miutil {


namespace socket_status_codes {
typedef enum  {
  NO_ERR = 0, 
  SOCKET_CREATE_ERR, 
  SOCKET_BIND_ERR,
  SOCKET_ACCEPT_ERR,
  SOCKET_LISTEN_ERR,
  SET_SOCKOPT_ERR,
  GET_HOSTBYNAME_ERR,
  CONNECT_ERR,
  NOT_CONNECTED_ERR,
  NO_MEM
} ErrorTypes;
}



/**
 * readString
 * Read a line from a descriptor.  Read the line one byte at a time,
 * looking for the newline.  We store the newline in the buffer.
 * 
 * We return the number of characters read, on success and -1 on error.
 *
 * \param fd the descriptor to read from.
 * \param str the buffer to hold the data.
 * \param timeoutInSec This set the allowed timout between each byte that is
 *                     received.
 *
 * \return -1 on error, number of byte read on success (including the newline
 *           character).
 * Based on code from UNIX Network Programming
 */

int 
readString(int fd, std::string &str, int timeoutInSec);


int
readStringExt(int fd, std::string &str, int timeoutInMillimSec);

int
socketGetCh(int fd, char &ch, bool &timedout, int timeoutInSec);

int
socketGetChExt(int fd, char &ch, bool &timedout, int timeoutInMillimSec);


/**
 * writeString 
 * Write a string to a descriptor. If the buffer does'nt end in a 
 * newline, a newline is appenden on write. The buffer can contain more 
 * than one line (ie more than one newline), all lines will be written 
 * to the descriptor.
 * 
 * We return the number of characters written up to.
 * 
 * \param fd the descriptor to write to.
 * \param str write from this buffer.
 *
 * \return -1 on error, number of byte written on success. This can be more
 *            than the size of str since a newline may be added.
 *
 * Based on code from UNIX Network Programming
 */
int 
writeString(int fd, const std::string &str);

int socketWriteBuf(int fd, const std::string &str);

/**
 * socketSetLinger, 
 * Lingers on a close() if data is present. This option controls the 
 * action taken when unsent messages queue on a socket and close() 
 * is performed. If SO_LINGER is  set, the system blocks the process 
 * during close() until it can transmit the data or until the time expires. 
 * If SO_LINGER is not specified, and close() is issued, the
 * system handles the call in a way that allows the process to continue as 
 * quickly as possible. 
 * 
 * \param sd the socket.
 * \param enable are we enabling or disabling linger for the socket.
 * \param timeoutInSec how long shall we wait for the data to be sendt
 *        before we close anyway.
 *
 * \return true on success, false otherwise.
 */ 

bool
socketSetLinger(int sd, bool enable, int timeoutInSec);

/**
 * setupConnection setups a connection to the server 'servername', using
 * the port 'port'.
 *
 * \param servername the name of the machine the server is running on.
 * \param port the port number to the service.
 *
 * \return NO_ERR on success, on error see the enum ErrorTypes.  
 */

socket_status_codes::ErrorTypes
setupConnection(const std::string &servname, int port, int& sd);
}
#endif

