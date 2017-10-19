#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_sec.h"

zend_class_entry *sec_ce;
PHP_METHOD(antiFilter, xss_clean )
{
    std::string test=antiFilter::remove_xss("http://%77%77%77%2E%67%6F%6F%67%6C%65%2E%63%6F%6D<span style='color:#0066>00;'>");
    //php_printf(test.c_str());
}
static zend_function_entry filter_method[]={
    PHP_ME(antiFilter, xss_clean,  NULL,   ZEND_ACC_PUBLIC)
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
    "anti_filter",
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
#ifdef COMPILE_DL_ANTI_FILTER
ZEND_GET_MODULE(sec)
#endif
