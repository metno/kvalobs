#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../kvglue/kvglue.h"

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int len, int arg)
{
    errno = EINVAL;
    return 0;
}

MODULE = KvService		PACKAGE = KvService		


double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL


SV *
getParams()
   INIT:
	AV           *result;
	kvsParamList *list;

	result=(AV*)sv_2mortal((SV*)newAV());
   CODE:
	list=kvsGetParams();
	
	while(list){
	  HV * rh;
	
	  printf("Param: %3s %-3s (%d) [%d]\n", 
	   list->param->name, list->param->unit, list->param->paramID,
	   list->param->level_scale);
	  
	  rh=(HV*)sv_2mortal((SV*)newHV());
	  
	  hv_store(rh, "paramID", 7, newSViv(list->param->paramID), 0);
          hv_store(rh, "level_scale", 11, newSViv(list->param->level_scale), 0);
	  hv_store(rh, "name", 4, newSVpv(list->param->name,strlen(list->param->name)),0);
	  hv_store(rh, "unit", 4, newSVpv(list->param->unit, strlen(list->param->unit)),0);
	  list=kvsPopParamList(list); 
	  av_push(result, newRV((SV*)rh));
        }

	RETVAL=newRV((SV*)result);
   OUTPUT:
	RETVAL 

SV*
subscribeDataNotify(callback)
    SV * callback;
    
    INIT:
       SV * id;
              
       id=&PL_sv_undef;       
    CODE:
	
	if(SvROK(callback)){
	   printf("callback:  REF\n");

	   if(SvTYPE(SvRV(callback))==SVt_PVCV){
	      printf("callback: CODE\n");
           }else{
	      printf("Must be an referance to a subroutine!\n");
           }
        }else{
	   printf("Must be an referance to a subroutine!\n");
        }
     
	id=(SV*)newSVpv("hei der!", 0);
    
       RETVAL=id;
    OUTPUT:
       RETVAL