#include "php_sec.h"
zend_class_entry *sec_ce;
void remove_invisible_characters(char *str,int str_len,bool url_encode){
    zval *non_displayables;
    int i=0;
    zval func,*params[5],count,retval;
    MAKE_STD_ZVAL(non_displayables);
    MAKE_STD_ZVAL(params[1]);
    MAKE_STD_ZVAL(params[2]);
    MAKE_STD_ZVAL(params[3]);
    array_init(non_displayables);
    if(url_encode){
        add_index_string(non_displayables,i,"/%0[0-8bcef]/i",1);
        i++;
        add_index_string(non_displayables,i,"/%1[0-9a-f]/i",1);
        i++;
        add_index_string(non_displayables,i,"/%7f/i",1);
        i++;
    }
    add_index_string(non_displayables,i,"/[\x00-\x08\x0B\x0C\x0E-\x1F\x7F]+/S",1); 
    ZVAL_STRING(&func, "preg_replace", 0);
    params[0]=non_displayables;
    INIT_PZVAL(params[0]);
    ZVAL_STRING(params[1], "", 0);
    ZVAL_STRINGL(params[2],str,str_len, 0);
    ZVAL_LONG(params[3],-1);
    params[4]=&count;
    INIT_PZVAL(params[4]);
    do{
       call_user_function(EG(function_table),NULL,&func,&retval,1,params); 
    }while(Z_LVAL(count));
}
PHP_METHOD(sec, xss_clean )
{
    char *str;
    int str_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS(),"s",&str,&str_len)==FAILURE){
        return;
    }
    remove_invisible_characters(str,str_len,true);
    //str will be destory because it is param,so set dup is 1
    RETURN_STRING(str,1);
}
static zend_function_entry filter_method[]={
    PHP_ME(sec, xss_clean,  NULL,   ZEND_ACC_PUBLIC)
    {NULL,NULL,NULL}

};
PHP_MINIT_FUNCTION(sec)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce,"sec",filter_method);
    sec_ce=zend_register_internal_class(&ce);
    return SUCCESS;
}
//module entry
zend_module_entry sec_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
     STANDARD_MODULE_HEADER,
#endif
    "sec",
    filter_method, /* Functions */
    PHP_MINIT(sec),/* MINIT */
    NULL, /* MSHUTDOWN */
    NULL, /* RINIT */
    NULL, /* RSHUTDOWN */
    NULL, /* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
    "0.1",
#endif
    STANDARD_MODULE_PROPERTIES
};
#ifdef COMPILE_DL_SEC
ZEND_GET_MODULE(sec)
#endif
