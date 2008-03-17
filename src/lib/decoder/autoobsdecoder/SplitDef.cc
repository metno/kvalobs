/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SplitDef.cc,v 1.10.6.2 2007/09/27 09:02:24 paule Exp $                                                       

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
#include "convert.h"

namespace kvalobs{
  namespace decoder{
    namespace autoobs{
      
      SplitDef _irix[]={{"IR", 0, 1},
			{"IX", 1, 1},
			{0, 0, 0}};
      
      //Sjekk denne
      SplitDef _RRRtr[]={{"_RRR", 0, 3},
			 {"_tr", 3, 1},
			 {0, 0, 0}};
      
      SplitDef _hVV[]={{"HL", 0, 1},
		       {"VV", 1, 2},
		       {0, 0, 0}};

      SplitDef _h[]={{"HL", -1, -1},
		     {0,0,0}};
      SplitDef _VV[]={{"VV", -1,-1},
		      {0,0,0}};
      
      SplitDef _wwW1W2[]={{"WW", 0, 2},
			  {"W1", 2, 1},
			  {"W2", 3, 1},
			  {0, 0, 0}};
      
      SplitDef _RtWdWdWd[]={{"_Rt", 0, 1},
			    {"X1WD", 1, 1},
			    {"X2WD", 2, 1},
			    {"X3WD", 3, 1},
			    {0, 0, 0}};

      SplitDef _WdWdWd[]={{"X1WD", 0, 1},
			  {"X2WD", 1, 1},
			  {"X3WD", 2, 1},
			  {0, 0, 0}};

      
      
      SplitDef _NhClCmCh[]={{"NH", 0, 1},
			    {"CL", 1, 1},
			    {"CM", 2, 1},
			    {"CH", 3, 1},
			    {0, 0, 0}};
      
      SplitDef _1NsChshs[]={{"NS1", 0, 1},
			    {"CC1", 1, 1},
			    {"HS1", 2, 2},
			    {0, 0, 0}};

      SplitDef _2NsChshs[]={{"NS2", 0, 1},
			    {"CC2", 1, 1},
			    {"HS2", 2, 2},
			    {0, 0, 0}};

      SplitDef _3NsChshs[]={{"NS3", 0, 1},
			    {"CC3", 1, 1},
			    {"HS3", 2, 2},
			    {0, 0, 0}};

      SplitDef _4NsChshs[]={{"NS4", 0, 1},
			    {"CC4", 1, 1},
			    {"HS4", 2, 2},
			    {0, 0, 0}};

      
      SplitDef _dd[]={{"DD", -1, -1},
		      {0, 0, 0}};
      
      SplitDef _ff[]={{"FF", -1, -1},
		      {0, 0, 0}};
      
      SplitDef _fg[]={{"FG", -1, -1},
		      {0, 0, 0}};
      
      SplitDef _fx[]={{"FX", -1, -1},
		      { 0, 0, 0}};
      SplitDef _SD[]={{"SD", -1, -1},
		      { 0, 0, 0}};
      
      SplitDef _tz[]={{"ITZ", -1, -1},
		      {0, 0, 0}};
      
      SplitDef _snTwTwTw[]={{"TW", -1, -1},
			    {0,0,0}};
      
      /* Er dette riktig eller er p�f�lgende riktig.
      SplitDef _VT_old1[]={{"V7", 0, 2},
			   {"V6", 2, 2},
			   {0, 0, 0}};
      
      SplitDef _VT_old2[]={{"V5", 0, 2},
			   {"V4", 2, 2},
			   {0, 0, 0}};
      
      SplitDef _VT_new1[]={{"", 0, 2},
			   {"V3", 2, 2},
			   {0, 0, 0}};
      
      SplitDef _VT_new2[]={{"V2", 0, 2},
			   {"V1", 2, 2},
			   {0, 0, 0}};
      */

      SplitDef _VT_old1[]={{"V4", 0, 2},
			   {"V5", 2, 2},
			   {0, 0, 0}};
      
      SplitDef _VT_old2[]={{"V6", 0, 2},
			   {"V7", 2, 2},
			   {0, 0, 0}};
      
      SplitDef _VT_new1[]={{"V1", 0, 2},
			   {"V2", 2, 2},
			   {0, 0, 0}};
      
      SplitDef _VT_new2[]={{"V3", 0, 2},
			   {"", 2, 2},
			   {0, 0, 0}};
      
      SplitDef _aa[]={{"AA", -1, -1},
		      {0, 0, 0}};
      
      SplitDef _N[]={{"NN", 0, 1},
		     {0, 0, 0}};
      
      SplitDef _S[]={{"SG", 0, 1},
		     {0, 0, 0}};
      
      SplitDef _Esss[]={{"EM", 0, 1},
			{"SA", 1, 3},
			{0, 0, 0}};
      
      
      SplitData splitArray[]={ {"_irix", _irix},
			       {"_RRRtr", _RRRtr},
			       {"_hVV", _hVV},
			       {"_h",   _h},
			       {"_VV",  _VV},
			       {"_wwW1W2", _wwW1W2},
			       {"_RtWdWdWd", _RtWdWdWd},
			       {"_WdWdWd", _WdWdWd},
			       {"_NhClCmCh", _NhClCmCh},
			       {"_NsChshs",  _1NsChshs},
			       {"_1NsChshs", _1NsChshs},
			       {"_2NsChshs", _2NsChshs},
			       {"_3NsChshs", _3NsChshs},
			       {"_4NsChshs", _4NsChshs},
			       {"_VT_old1", _VT_old1},
			       {"_VT_old2", _VT_old2},
			       {"_VT_new1", _VT_new1},
			       {"_VT_new2", _VT_new2},
			       {"aa", _aa},
			       {"_aa", _aa},
			       {"_S", _S},
			       {"_N", _N},
			       {"_Esss", _Esss},
			       {"_dd", _dd},
			       {"_ff", _ff},
			       {"_fg", _fg},
			       {"_fx", _fx},
			       {"_SD", _SD},
			       {"_tz", _tz},
			       {"_snTwTwTw", _snTwTwTw},
			       {0, 0}};
    }
  }
}

