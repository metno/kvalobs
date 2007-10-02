/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DbConnectionTest.java,v 1.1.2.2 2007/09/27 09:02:44 paule Exp $                                                       

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

import junit.framework.*;
import metno.dbutil.*;

public class DbConnectionTest extends TestCase{
    static final String dbdriver="org.hsqldb.jdbcDriver";
    static final String dbconnect="jdbc:hsqldb:tmp/testdb";
    static final String dbpasswd="";
    static final String dbuser="sa"; //Default user in a HSQLDB.
    
    boolean ok;
    DbConnection dbcon;
    
    public void setUp(){
        
        try {
            Class.forName(dbdriver);
            System.out.println("Drive <"+dbdriver+"> loaded!");
            ok=true;
        }catch(Exception e) {
            System.out.println("FATAL: cant load the database driver <"+
                               dbdriver+">!");
            ok=false;
            return;
        }
    
       
        Connection con=null;
        Statement statement;

        try{
            con = DriverManager.getConnection(dbconnect, 
                                              dbuser, 
                                              dbpasswd);
            statement=con.createStatement();
            ok=true;
        }
        catch(SQLException ex){
            System.out.println(ex);
            System.out.println("FATAL: cant create a database"+
                                   "connection <"+dbconnect+">!");
                
            if(con!=null){
                try{
                    con.close();
                }
                catch(Exception ex_){
                }
            }
            
            ok=false;
            return;
        }
       
        dbcon=new DbConnection(con, statement, dbdriver);
    }
    
    public void tearDown(){
        if(!ok)
            return;
        
        try{
            dbcon.statement.execute("SHUTDOWN");
            dbcon.statement.close();
            dbcon.statement=null;
            dbcon.connection.close();
            dbcon.connection=null;
            dbcon=null;
        }
        catch(Exception ex){
        }
    }
    
    public void testConstruction(){
        if(!ok){
            throw new AssertionFailedError("DbConnection not initialized!");
        }

        assertNotNull(dbcon.connection);
        assertNotNull(dbcon.statement);
        assertNotNull(dbcon.dbdriver);
        assertNotNull(dbcon.created);
        assertNotNull(dbcon.lastAccess);
        assertTrue(dbcon.inuse);
    }
    
    public void testIllegalStateException(){
        if(!ok){
            throw new AssertionFailedError("DbConnection not initialized!");
        }
        
        assertTrue(dbcon.getInuse());
        
        dbcon.setInuse(false);
        
        assertFalse(dbcon.getInuse());
        
        try{
            dbcon.exec("");
            fail("EXCEPTION expected: IllegalStateException!");
        }
        catch(IllegalStateException ex){
        }
        catch(SQLException e){
            fail("Unexpected EXCEPTION: "+e.getMessage());
        }
        
        try{
            dbcon.execQuery("");
            fail("EXCEPTION expected: IllegalStateException!");
        }
        catch(IllegalStateException ex){
        }
        catch(SQLException e){
            fail("Unexpected EXCEPTION: "+e.getMessage());
        }
        
    }
}