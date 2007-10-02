#include <stdio.h>
#include "kvglue.h"


int
main(int argn, char **argv)
{
  kvsParamList *list;

  list=kvsGetParams();

  if(!list){
    printf("problems 1\n");
    return 1;
  }
  
  do{
    printf("Param: %3s %-3s (%d) [%d]\n", 
	   list->param->name, list->param->unit, list->param->paramID,
	   list->param->level_scale);
    list=kvsPopParamList(list);
  }while(list);

  return 0;

}
