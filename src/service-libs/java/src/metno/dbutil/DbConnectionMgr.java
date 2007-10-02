/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DbConnectionMgr.java,v 1.1.2.8 2007/09/27 09:02:42 paule Exp $                                                       

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
package metno.dbutil;

import metno.util.MiGMTTime;
import java.sql.*;
import java.util.*;

public class DbConnectionMgr
{
    String dbconnect;
    String dbdriver;
    String dbuser;
    String dbpasswd;
    DbConnection[] dbconCache;
    boolean isShutdown;
    int     timeToLive; //How long shall a connection live before it is deleted.
    int     idleTime;   //How long shall a connection be allowed to be inactive
                        //before we delete it.

    /**
     * Instatiate an connectionmanager whit connection data 
     * from the propperties. The following properties are searcehed 
     * for. 
     * 
     * <pre>
     *  * dbdriver - The database driver to use.
     *    dbuser   - The user to use to create a connection.
     *    dbpasswd - The password to use to create a connection.
     *  * dbconnect- The connectstring to use to create a connection.
     *    dbmaxconnections - Max connection to cache.
     * </pre>
     * 
     * Properties marked with * above is mandatory.
     * @throws ClassNotFoundException When the database driver cant 
     *                                be loaded. 
     * @throws IllegalArgumentException If driver or connectstring is missing.
     */
    public DbConnectionMgr(Properties prop)
        throws ClassNotFoundException,IllegalArgumentException{
        this(prop.getProperty("dbdriver"),
             prop.getProperty("dbuser", ""),
             prop.getProperty("dbpasswd", ""),
             prop.getProperty("dbconnect"),
             Integer.parseInt(prop.getProperty("dbmaxconnections", "1")));
    }
    
    
    /**
     *  Convenient constructor to use when most of the connection
     *  properties may not be given. Only the mandatory propertis
     *  is given.
     *   
     * @param dbdriver The database driver to use.
     * @param dbconnectstr The connectstring to use to create a connection.
     * @param maxConnections Max connections to cache.
     * @throws ClassNotFoundException When the database driver cant 
     *                                be loaded. 
     * @throws IllegalArgumentException If driver or connectstring is missing.
     */
    public DbConnectionMgr(String dbdriver, String dbconnectstr, 
                           int maxConnections)
        throws ClassNotFoundException,IllegalArgumentException{
        this(dbdriver, "", "", dbconnectstr, maxConnections);
    }
    
    /**
     * 
     * @param dbdriver The database driver to use.
     * @param dbuser The user to use to create a connection.
     * @param dbpasswd The password to use to create a connection.
     * @param dbconnectstr The connecttring to use to create a connection.
     * @param maxConnections Max connection to cache, default 1.
     * @throws ClassNotFoundException When the database driver cant 
     *                                be loaded. 
     * @throws IllegalArgumentException If driver or connectstring is missing.
     */
    public DbConnectionMgr(String dbdriver, 
                           String dbuser,
                           String dbpasswd,
                           String dbconnectstr,
                           int    maxConnections)
        throws ClassNotFoundException,IllegalArgumentException{

        if(dbdriver==null)
            throw new IllegalArgumentException("DbConnectionMgr: dbdriver==0.");
        
        this.dbdriver=dbdriver.trim();
        
        if(this.dbdriver.length()==0)
            throw new IllegalArgumentException("DbConnectionMgr: dbdriver is an empty string!");
        
     
        if(dbconnectstr==null)
            throw new IllegalArgumentException("DbConnectionMgr: dbconnectstr==0.");
        
        this.dbconnect=dbconnectstr.trim();
        
        if(this.dbconnect.length()==0)
            throw new IllegalArgumentException("DbConnectionMgr: dbconnectstr is an empty string!");
     
        if(dbuser==null)
            this.dbuser="";
        else
            this.dbuser=dbuser.trim();
            
        if(dbpasswd==null)
            this.dbpasswd="";
        else
            this.dbpasswd=dbpasswd.trim();

        if(maxConnections<=0)
            throw new IllegalArgumentException("DbConnectionMgr: maxConnections<=0!");
        
        Class.forName(this.dbdriver);
        
        dbconCache=new DbConnection[maxConnections];
        
        isShutdown=false;
        timeToLive=-1;
        idleTime=-1;
    }
    
    protected void finalize() throws Throwable {
        closeDbDriver();
    }
                                    
    /**
     * This function is used to do some last minute shutdown statement
     * for the database engine used. What to do is specific to each 
     * database engine. 
     * 
     * All connections that is NOT in use is closed! It is an ERROR
     * to call this method if there is still connections in use.
     * 
     * At the moment it is only HSQLDB (http://hsqldb.org) that use this.
     * 
     * Tested database engines is postgresql, oracle and HSQLDB.
     */
     synchronized public void closeDbDriver(){
        if(isShutdown)
            return;
        
        DbConnection con;
        isShutdown=true;
        
        if(dbdriver.compareTo("org.hsqldb.jdbcDriver")==0){
            try{
                con=realNewDbConnection();
            }
            catch(SQLException ex){
                //ex.printStackTrace();
                con=null;
            }
            
            if(con!=null){
                try {
                    con.exec("SHUTDOWN");
                } catch (IllegalStateException e) {
                    e.printStackTrace();
                } catch (SQLException e) {
                    e.printStackTrace();
                }
                
                try {
                    con.close();
                } catch (SQLException e) {
                    e.printStackTrace();
                }
            }
        }
        
        for(int i=0; i<dbconCache.length; i++){
            if(dbconCache[i]!=null){
                if(dbconCache[i].getInuse()){
                    System.out.println("WARNING: Shuting down the DbConnectionMgr, but connections still in use!");
                }else{
                    try {
                        dbconCache[i].close();
                    } catch (SQLException e) {
                        e.printStackTrace();
                    }
                    dbconCache[i]=null;
                }                    
            }
        }
     }   
    
     DbConnection realNewDbConnection()
         throws SQLException{
         Connection con=null;
         Statement statement=null;

         try{
             con = DriverManager.getConnection(dbconnect, 
                                               dbuser, 
                                               dbpasswd);
             
             if(!con.getAutoCommit())
                 con.setAutoCommit(true);
                          
             statement=con.createStatement();
                          
         }
         catch(SQLException ex){
             if(con!=null){
                 try{
                     con.close();
                 }
                 catch(Exception ex_){
                 }   
             }
             
             throw ex;
         }
         
         return new DbConnection(con, statement, dbdriver);
     }

     
     /**
      * This method is the same as newDbConnection(int timeoutInSec), but with
      * timeoutInSeconds set to 0, ie wait without limit to the completion.
      * 
      * @return A DbConnection.
      * @throws SQLException
      */
     synchronized public DbConnection newDbConnection()
     	throws SQLException{
    	 return newDbConnection(0);
     }
     
     /**
      * Get a new DbConnections. The connections is either taken from 
      * the cache or a new one is created.
      * 
      * The connection must be released with a call to releaseDbConnection.
      * 
      * Null is returned if maxconnections is in use and no IdleConnections
      * is in the cache.
      * 
      * The returned connection is in AutoCommit state. I one want to run in
      * transaction mode a call to setAutoCommit(false) must be issud on the
      * connection.
      *
      * @param timeoutInSec Throw an exception if a operation is not completed in 
      *                      timeoutInSec seconds. 
      * @return A new DbConnection or null if error.
      * @throws SQLException If we could'nt create a new connection to the database.
      */
     synchronized public DbConnection newDbConnection(int timeoutInSec)
         throws SQLException{
         
         checkForConnectionsToClose();
         
         DbConnection dbcon=null;
         int nullSlot=-1;
        
         for(int i=0; dbcon==null && i<dbconCache.length; i++){
            if(dbconCache[i]==null){
                if(nullSlot==-1)
                    nullSlot=i;
            }else if(!dbconCache[i].getInuse()){
                dbcon=dbconCache[i];
            }
        }   

        if(dbcon!=null){
            dbcon.setInuse(true);
            dbcon.setTimeout(timeoutInSec);
            return dbcon;
        }
        
        if(nullSlot!=-1){
            dbcon=realNewDbConnection();
            dbcon.setTimeout(timeoutInSec);
            dbconCache[nullSlot]=dbcon;
            return dbcon;
        }
        
        
        
        return null;
    }

     
    synchronized public void  releaseDbConnection(DbConnection con)
        throws SQLException, IllegalArgumentException, IllegalStateException{
        DbConnection dbcon=null;

        if(con==null)
            throw new IllegalArgumentException("DbConnectionMgr.releaseDbConnection: Expected con!=null!");
        
        if(!con.getInuse())
            throw new IllegalStateException("Connection allready released!");

        for(int i=0; i<dbconCache.length; i++){
            if(dbconCache[i]==con){
                dbcon=con;
                try{
                    if(!dbcon.getAutoCommit())
                        dbcon.setAutoCommit(true);
                }
                catch(SQLException ex){
                    System.out.println("DbConnectionMgr.releaseDbConnection: Exception in get/setAutoCommit: "+ex.getMessage());
                    try{
                        dbcon.close();
                    }
                    catch(SQLException e){
                        System.out.println("DbConnectionMgr.releaseDbConnection: Exception (get/setAutoCommit) in close  : "+e.getMessage());
                    }
                    dbconCache[i]=null;
                    checkForConnectionsToClose();
                    return;
                }
                dbconCache[i]=dbcon;
                break;
            }
        }
        
        if(dbcon==null)
            throw new IllegalArgumentException("Connection was not created by this DbConnectionMgr!");

        try{
            if(!dbcon.getAutoCommit())
                dbcon.setAutoCommit(true);
        }
        catch(SQLException ex){
            
        }
        
        dbcon.setInuse(false);
        
        checkForConnectionsToClose();
    }
    
    public String getDbconnect(){
        return dbconnect;
    }
    public String getDbdriver(){
        return dbdriver;
    }
    public String getDbuser(){
        return dbuser;
    }

    public void setDbuser(String user){
        dbuser=user;
    }
    
    public void setDbpasswd(String pwd){
        dbpasswd=pwd;
    }

    public int getMaxconnections(){
        return dbconCache.length;
    }
    
    /**
     * Returns the count of all connections that is in use.
     *  
     * @return connections in use.
     */
    synchronized public int getConnectionsInuse(){
        int cnt=0;
        
        for(int i=0; i<dbconCache.length; i++){
            if(dbconCache[i]!=null && dbconCache[i].getInuse())
                cnt++;
        }
        
        return cnt;
    }
    
    /**
     * Idle connections is connections that is active but not
     * in use, ie they are cached for reuse.
     * @return
     */
    synchronized public int getIdleConnections(){
        int cnt=0;
        
        for(int i=0; i<dbconCache.length; i++){
            if(dbconCache[i]!=null && !dbconCache[i].getInuse())
                cnt++;
        }
        
        return cnt;
    }
    
    public void setIdleTime(int idleTimeInSeconds){
        idleTime=idleTimeInSeconds;
    }
    
    public void setTimeToLive(int timeToLiveInseconds){
        timeToLive=timeToLiveInseconds;
    }
    
    public int getIdleTime(){
        return idleTime;
    }
    
    public int getTimeToLive(){
        return timeToLive;
    }
    
    synchronized public int checkForConnectionsToClose(){
        int cnt=0;
        MiGMTTime now=new MiGMTTime();
        
        if(timeToLive>=0){
            for(int i=0; i<dbconCache.length; i++){
                if(dbconCache[i]!=null && !dbconCache[i].getInuse()){
                    MiGMTTime tmp=new MiGMTTime(dbconCache[i].getCreated());
                    
                    tmp.addSec(timeToLive);
                    System.out.println("timeToLive: secsTo: "+tmp.secsTo(now));
                    
                    if(tmp.secsTo(now)>=0){
                        cnt++;
                        try {
                            dbconCache[i].close();
                        } catch (SQLException e) {
                            e.printStackTrace();
                        }
                        dbconCache[i]=null;
                    }
                }
            }
        }

        if(idleTime>=0){
            for(int i=0; i<dbconCache.length; i++){
                if(dbconCache[i]!=null && !dbconCache[i].getInuse()){
                    MiGMTTime tmp=new MiGMTTime(dbconCache[i].getLastAccess());
                    
                    tmp.addSec(idleTime);
                    System.out.println("idleTime: secsTo: "+tmp.secsTo(now));

                    if(tmp.secsTo(now)>=0){
                        cnt++;
                        
                        try {
                            dbconCache[i].close();
                        } catch (SQLException e) {
                            e.printStackTrace();
                        }
                        dbconCache[i]=null;
                    }
                }
            }
        }

        return cnt;
    }
}
