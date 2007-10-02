/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DbTestUtil.java,v 1.1.2.2 2007/09/27 09:02:20 paule Exp $                                                       

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
package metno;

import java.io.File;
import java.sql.ResultSet;

import metno.dbutil.DbConnection;
import metno.dbutil.DbConnectionMgr;
import metno.util.FileUtil;

public class DbTestUtil {

	static public boolean listDbTable(DbConnectionMgr mgr, String tablename){
    	DbConnection con=null;
        
        try{
            con=mgr.newDbConnection();
            
            ResultSet rs=con.execQuery("Select * FROM "+tablename);
            int c=rs.getMetaData().getColumnCount();
            int n=0;
            
            String val;
            
            if(c>0)
            	System.out.println(rs.getMetaData().getTableName(1));
            
            for(int i=1; i<=c; i++){
            	val=rs.getMetaData().getColumnLabel(i);
            	n+=val.length();

                if(i>1){
                    System.out.print(",");
                    n++;
                }

                System.out.print(val);
            }
            
            System.out.println();
            
            for(int i=0; i<n; i++)
            	System.out.print("-");
            
            System.out.println();
            
            while(rs.next()){
                for(int i=1; i<=c; i++){
                    if(i>1)
                        System.out.print(",");
                    val=rs.getString(i);
                    
                    if(val==null)
                        System.out.print("NULL");
                    else
                        System.out.print(val);
                }
                System.out.println();
            }
            
            for(int i=0; i<n; i++)
            	System.out.print("-");
            
            System.out.println();
            System.out.println();

            mgr.releaseDbConnection(con);
        }
        catch(Exception e){
            
            if(con!=null){
                try{
                    mgr.releaseDbConnection(con);
                }
                catch (Exception ex) {
                }
            }
            
            return false;
        }
        return true;
    }
	
	static public boolean runSqlFromFile(DbConnectionMgr mgr, String filename){
    	String buf=FileUtil.readFile2Str(filename);
    	
    	if(buf==null)
    		return false;
    	
    	DbConnection con=null;
        
        try{
            con=mgr.newDbConnection();
            con.exec(buf);
            mgr.releaseDbConnection(con);
            
            return true;
        }catch(Exception e){
        	e.printStackTrace(); 
        	if(con!=null){
        		try{
        			mgr.releaseDbConnection(con);
                }
                catch (Exception ex) {
                }
            }
                
            return false;
        }
    }
	
	
	static public boolean runSqlFromString(DbConnectionMgr mgr, String sqlstmt){
    	
    	DbConnection con=null;
        
        try{
            con=mgr.newDbConnection();
            con.exec(sqlstmt);
            mgr.releaseDbConnection(con);
            
            return true;
        }catch(Exception e){
        	e.printStackTrace(); 
        	if(con!=null){
        		try{
        			mgr.releaseDbConnection(con);
                }
                catch (Exception ex) {
                }
            }
                
            return false;
        }
    }
    
	static public void deleteDb(String path){
    	File dbdir=new File(path);
        
        if(dbdir.exists()){
            if(dbdir.isDirectory()){
                String[] files=dbdir.list();
                int dirs=0;
                
                for(int i=0; i<files.length; i++){
                    File f=new File(path+"/"+files[i]);
                    if(f.isFile()){
                        f.delete();
                    }else{
                        dirs++;
                    }
                }
                
                if(dirs==0)
                    dbdir.delete();
            }
        }
    }
}
