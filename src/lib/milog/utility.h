
#ifndef __MILOG_UTILITY_H__
#define __MILOG_UTILITY_H__

namespace milog {


/**
 * Creates an globale logger.
 *
 * The name of the logfile wil bee 'path/prefix_id.log'.
 * @param path The path to write the logfile too.
 * @param prefix The prefix in the logfile name.
 * @param id The id the logger is known by.
 * @param ll The loglevel.
 * @param size The maximum size of the logfile in kilo bytes.
 * @param nFiles Number of logfiles backed up before they are deleted.
 * @return true on success and false otherwise.
 */
bool
createGlobalLogger( const std::string &path,
                    const std::string &prefix,
                    const std::string &id,
                    milog::LogLevel ll,
                    int size=200,
                    int nFiles=2);
}


#endif /* UTILITY_H_ */
