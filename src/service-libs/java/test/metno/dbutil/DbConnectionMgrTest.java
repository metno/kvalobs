/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DbConnectionMgrTest.java,v 1.1.2.3 2007/09/27 09:02:44 paule Exp $                                                       

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

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;

import junit.framework.*;
import metno.dbutil.*;

public class DbConnectionMgrTest extends TestCase{
    static final String dbdriver="org.hsqldb.jdbcDriver";
    static final String dbconnect="jdbc:hsqldb:file:tmp/testdb";
    static final String dbpasswd="";
    static final String dbuser="sa"; //Default user in a HSQLDB.
    
    boolean ok;
    
    public void setUp(){
    }
    
    public void tearDown(){
    }    

    //Count null referances in an DbConnectionMgr.dbconCache
    public int countNull(DbConnectionMgr mgr){
        int cntNull=0;

        for(int i=0; i<mgr.dbconCache.length; i++)
            if(mgr.dbconCache[i]==null)
                cntNull++;

        return cntNull;
    }

    public int countAutoCommit(DbConnectionMgr mgr)
        throws SQLException{
        int cnt=0;

        for(int i=0; i<mgr.dbconCache.length; i++){
            if(mgr.dbconCache[i]!=null){
                if(mgr.dbconCache[i].connection.getAutoCommit())
                    cnt++;
            }
        }
    
        return cnt;
    }

    
    public void sleep(int second){
        try {
            Thread.sleep(1000*second);
        } catch (InterruptedException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    
    public void testConstruction(){
        DbConnectionMgr mymgr=null;
        
        try {
            mymgr=new DbConnectionMgr(dbdriver, dbuser, dbpasswd, dbconnect, 2);
        }catch(Exception e) {
            fail(e.getMessage());
        }

        assertFalse(mymgr.isShutdown);
        
        assertEquals(dbdriver, mymgr.getDbdriver());
        assertEquals(dbuser, mymgr.getDbuser());
        assertEquals(dbpasswd, mymgr.dbpasswd);
        assertEquals(dbconnect, mymgr.getDbconnect());
        assertEquals(2, mymgr.getMaxconnections());
        
        mymgr.closeDbDriver();
        assertTrue(mymgr.isShutdown);
        mymgr=null;

        
        Properties prop=new Properties();
        
        prop.setProperty("dbdriver", dbdriver);
        prop.setProperty("dbuser", dbuser);
        prop.setProperty("dbpasswd", "mypass");
        prop.setProperty("dbconnect", dbconnect);
        prop.setProperty("dbmaxconnections", "2");

        try {
            mymgr=new DbConnectionMgr(prop);
        }catch(Exception e) {
            fail(e.getMessage());
        }
        
        assertEquals(dbdriver, mymgr.getDbdriver());
        assertEquals(dbuser, mymgr.getDbuser());
        assertEquals("mypass", mymgr.dbpasswd);
        assertEquals(dbconnect, mymgr.getDbconnect());
        assertEquals(2, mymgr.getMaxconnections());

        mymgr.closeDbDriver();
        mymgr=null;

        try {
            mymgr=new DbConnectionMgr("", dbuser, dbpasswd, dbconnect, 2);
            mymgr.closeDbDriver();
            mymgr=null;
            fail("Expected IllegalArgumentException");
        }
        catch(IllegalArgumentException e){
        }
        catch(Exception e) {
            fail(e.getMessage());
        }

        try {
            mymgr=new DbConnectionMgr(null, dbuser, dbpasswd, dbconnect, 2);
            mymgr.closeDbDriver();
            mymgr=null;

            fail("Expected IllegalArgumentException");
        }
        catch(IllegalArgumentException e){
        }
        catch(Exception e) {
            fail(e.getMessage());
        }

        try {
            mymgr=new DbConnectionMgr(dbdriver, dbuser, dbpasswd, "", 2);
            mymgr.closeDbDriver();
            mymgr=null;

            fail("Expected IllegalArgumentException");
        }
        catch(IllegalArgumentException e){
        }
        catch(Exception e) {
            fail(e.getMessage());
        }
        

        try {
            mymgr=new DbConnectionMgr(dbdriver, dbuser, dbpasswd, null, 2);
            mymgr.closeDbDriver();
            mymgr=null;

            fail("Expected IllegalArgumentException");
        }
        catch(IllegalArgumentException e){
        }
        catch(Exception e) {
            fail(e.getMessage());
        }

        try {
            mymgr=new DbConnectionMgr(dbdriver, dbuser, dbpasswd, dbconnect, 0);
            mymgr.closeDbDriver();
            mymgr=null;

            fail("Expected IllegalArgumentException");
        }
        catch(IllegalArgumentException e){
        }
        catch(Exception e) {
            fail(e.getMessage());
        }

        try {
            mymgr=new DbConnectionMgr(dbdriver, dbconnect, 2);
        }
        catch(Exception e) {
            fail(e.getMessage());
        }

        assertEquals(dbdriver, mymgr.getDbdriver());
        assertEquals("", mymgr.getDbuser());
        assertEquals("", mymgr.dbpasswd);
        assertEquals(dbconnect, mymgr.getDbconnect());
        assertEquals(2, mymgr.getMaxconnections());

        mymgr.closeDbDriver();
    }
    
    public void testNewDbConnection(){
        DbConnectionMgr mgr=null;
        DbConnection    con=null;
        DbConnection    con1=null;
        DbConnection    con2=null;
        
        try {
            mgr=new DbConnectionMgr(dbdriver, 
                                    dbuser, 
                                    dbpasswd, 
                                    dbconnect, 
                                    2);
        }catch(Exception e) {
            fail(e.getMessage());
        }

        
        assertEquals("Count", 2, countNull(mgr));
        
        try{
            con=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }
        
        assertNotNull(con);

        assertEquals(1, countNull(mgr));

        try{
            con1=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        
        assertEquals(0, countNull(mgr));
        
        assertEquals(2, mgr.getConnectionsInuse());
        
        try{
            con2=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        assertNull(con2);
        
        try{
            mgr.releaseDbConnection(con1);
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }
        
        assertFalse(con1.getInuse());
        assertEquals(1, mgr.getConnectionsInuse());

        
        try{
            con2=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        assertNotNull(con2);

        assertEquals(2, mgr.getConnectionsInuse());

        try{
            mgr.releaseDbConnection(con);
            mgr.releaseDbConnection(con2);
            
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        assertEquals(0, mgr.getConnectionsInuse());
        
        mgr.closeDbDriver();
        
        assertEquals(2, countNull(mgr));
    }
    
    
    public void testCeckForConnectionsToClose(){
        DbConnectionMgr mgr=null;
        DbConnection    con=null;
        DbConnection    con1=null;
        DbConnection    con2=null;
                
        try {
            mgr=new DbConnectionMgr(dbdriver, 
                                    dbuser, 
                                    dbpasswd, 
                                    dbconnect, 
                                    2);
        }catch(Exception e) {
            fail(e.getMessage());
        }

        //Allocate two connections        
        try{
            con=mgr.newDbConnection();
            con1=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        //Check the Idle tim logic
        //Set idle to 4 second.
        
        mgr.setIdleTime(4);

        
        assertEquals(0, countNull(mgr));
        
        try{
            mgr.releaseDbConnection(con1);
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }
        
        sleep(2);
        
        assertEquals(0, countNull(mgr));
        
        mgr.checkForConnectionsToClose();
        
        assertEquals(0, countNull(mgr));
        
        sleep(3);
        
        mgr.checkForConnectionsToClose();
        
        assertEquals(1, countNull(mgr));
        
        //Check the timeToLiveLogic
        //First allocate one more connection
        
        mgr.setIdleTime(-1); //Disable idltime
        mgr.setTimeToLive(4); //Set timeToLive to 4 seconds.
        
        try{
            con1=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }
                
        assertEquals(0, countNull(mgr));
        
        try{
            mgr.releaseDbConnection(con1);
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        sleep(2);
        
        assertEquals(0, countNull(mgr));
        
        mgr.checkForConnectionsToClose();
        
        assertEquals(0, countNull(mgr));
        
        sleep(3);
        
        mgr.checkForConnectionsToClose();
        
        assertEquals(1, countNull(mgr));
        
        //If timeToLive is zero shall all calls to releaseDbConnection
        //do a real release. No idle caching.
        
        mgr.setTimeToLive(0); //Set timeToLive to 0. No caching.

        try{
            con1=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }
                
        assertEquals(0, countNull(mgr));

        try{
            mgr.releaseDbConnection(con1);
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        assertEquals(1, countNull(mgr));

    }
    
    public void testCheckForAutoCommitAfterRelease(){
        DbConnectionMgr mgr=null;
        DbConnection    con=null;
        DbConnection    con1=null;
                
        try {
            mgr=new DbConnectionMgr(dbdriver, 
                                    dbuser, 
                                    dbpasswd, 
                                    dbconnect, 
                                    2);
        }catch(Exception e) {
            fail(e.getMessage());
        }

        //Allocate two connections        
        try{
            con=mgr.newDbConnection();
            con1=mgr.newDbConnection();
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }

        assertEquals(0, countNull(mgr));
        
        try{
            assertTrue(con.getAutoCommit());
            con.setAutoCommit(false);
            assertFalse(con.getAutoCommit());
            mgr.releaseDbConnection(con);
            assertEquals(2, countAutoCommit(mgr));
            mgr.releaseDbConnection(con1);
        }
        catch(IllegalStateException e){
            fail("Unexpected IllegalStateException: "+e.getMessage());
        }
        catch (SQLException e) {
            fail("Unexpected SQLException: "+e.getMessage());
        }
        
    }
}