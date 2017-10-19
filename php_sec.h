#ifndef PHP_SEC_H
#define PHP_SEC_H
extern zend_module_entry sec_module_entry;
//do not need ZTS or NZTS,because no use global variable
//learn from yaconf,it has declare a global variable by ZEND_BEGIN_MODULE_GLOBALS
PHP_MINIT_FUNCTION(sec);
PHP_MSHUTDOWN_FUNCTION(sec);
PHP_RINIT_FUNCTION(sec);
PHP_RSHUTDOWN_FUNCTION(sec);
PHP_MINFO_FUNCTION(sec);
#endif	/* PHP_SEC_H */
