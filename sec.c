#include "php_sec.h"
zend_class_entry *sec_ce;
zval* remove_invisible_characters(char *str,int str_len,bool url_encode){
    zval non_displayables;
    int i=0;
    zval func,*params[5],*count,*retval;
	//用栈空间进行初始化时，需要初始化引用值为1，因为call_user_function会对参数进行+1后-1(调用了Z_ADDREF_PP)，再判断是否为零，进行释放，如果不初始化引用值，则会被释放。
	//因为栈空间初始化的，如果被释放则引发段错误
	//array类型的释放会先释放ht再释放zval
    //MAKE_STD_ZVAL(non_displayables);
    INIT_ZVAL(non_displayables);
    MAKE_STD_ZVAL(params[1]);
    MAKE_STD_ZVAL(params[2]);
    MAKE_STD_ZVAL(params[3]);
    //init retval because function copy zval struct to retval derectly.
    MAKE_STD_ZVAL(retval);
    MAKE_STD_ZVAL(count);
	ZVAL_LONG(count, 0);
    array_init(&non_displayables);
    if(url_encode){
        add_index_string(&non_displayables,i,"/%0[0-8bcef]/i",0);
        i++;
        add_index_string(&non_displayables,i,"/%1[0-9a-f]/i",0);
        i++;
        add_index_string(&non_displayables,i,"/%7f/i",0);
        i++;
    }
    add_index_string(&non_displayables,i,"/[\\x00-\\x08\\x0B\\x0C\\x0E-\\x1F\\x7F]+/S",0);
    //stack store the address of "preg_replace",do not need to dup. 
    ZVAL_STRING(&func, "preg_replace", 0);
    params[0]=&non_displayables;
    ZVAL_STRING(params[1], "", 0);
    ZVAL_STRING(params[2],str, 1);
    //params[2]=retval;
    ZVAL_LONG(params[3],-1);
    params[4]=count;
    do{


	//will destory the params,retval is the new one ,not the make_std_zval created
        call_user_function(EG(function_table),NULL,&func,retval,5,params); 
		//只保留zvalue到params[2]，不释放retval，因为循环中要用到
		ZVAL_ZVAL(params[2],retval,0,0);
    }while(Z_LVAL_P(count));
	//手动释放ht
	FREE_HASHTABLE(Z_ARRVAL(non_displayables));
	//栈空间时不需要释放zval了。如果是堆空间，可以直接调用zval_ptr_dtor，会把ht的空间和zval结构体一并释放;zval_dtor只释放zvalue指针
	//FREE_ZVAL(non_displayables);//单独释放zval结构体，不涉及里面的指针部分
	FREE_ZVAL(params[1]);
	zval_dtor(params[3]);
	FREE_ZVAL(retval);//free retval
	zval_dtor(count);//free
    return params[2];
}
PHP_METHOD(sec, _urldecodespaces)
{
	zval *match,func,*params[3],**input,*nospaces;
	if(zend_parse_parameters(ZEND_NUM_ARGS(),"a",&match)==FAILURE){
        return;
    }
	zend_hash_index_find(Z_ARRVAL_P(match),0,(void **)&input);
	ZVAL_STRING(&func,"preg_replace",0);
	MAKE_STD_ZVAL(nospaces);
	MAKE_STD_ZVAL(params[0]);
    MAKE_STD_ZVAL(params[1]);
    MAKE_STD_ZVAL(params[2]);
	ZVAL_STRING(params[0],"#\\s+#",0);
	ZVAL_STRING(params[1],"",0);
	ZVAL_ZVAL(params[2],*input,0,0);
	call_user_function(EG(function_table),NULL,&func,nospaces,3,params);
	FREE_ZVAL(params[0]);
	FREE_ZVAL(params[1]);
	FREE_ZVAL(params[2]);
	if(strcmp(Z_STRVAL_P(nospaces),Z_STRVAL_PP(input))==0&&Z_TYPE_PP(input)==IS_STRING&&Z_TYPE_P(nospaces)==IS_STRING){
		//zval_dtor只释放zval中value申请的内存，并没有释放该zval调用MAKE_STD_ZVAL为该结构体生成的内存
		zval_dtor(nospaces);
		RETURN_ZVAL(*input,0,0);	
	}else{
		//zval_dtor(*input);
		RETURN_ZVAL(nospaces,0,0);
	}	
}
PHP_METHOD(sec, _convert_attribute)
{
    zval *match,func,*search,*replace,**input,retval,*params[3];
	int i=0;
    if(zend_parse_parameters(ZEND_NUM_ARGS(),"a",&match)==FAILURE){
        return;
    }
	zend_hash_index_find(Z_ARRVAL_P(match),0,(void **)&input);
	ZVAL_STRING(&func,"str_replace",0);
	MAKE_STD_ZVAL(search);
	array_init(search);
	MAKE_STD_ZVAL(replace);
	array_init(replace);
	add_index_string(search,i,">",0);
	add_index_string(replace,i,"&gt;",0);
	i++;
	add_index_string(search,i,"<",0);
	add_index_string(replace,i,"&lt;",0);
	i++;
	add_index_string(search,i,"\\",0);
	add_index_string(replace,i,"\\\\",0);
	i++;
    MAKE_STD_ZVAL(params[0]);
    MAKE_STD_ZVAL(params[1]);
    MAKE_STD_ZVAL(params[2]);
	ZVAL_ZVAL(params[0],search,0,0);
	ZVAL_ZVAL(params[1],replace,0,0);
	ZVAL_ZVAL(params[2],*input,0,0);
	call_user_function(EG(function_table),NULL,&func,&retval,3,params);
	FREE_ZVAL(params[0]);
    FREE_ZVAL(params[1]);
    FREE_ZVAL(params[2]);
	//因为数组的元素没有用堆申请空间进行拷贝，所以不能直接调用zval_ptr_dtor进行整体释放，只能手动释放
	FREE_HASHTABLE(Z_ARRVAL_P(search));
    FREE_ZVAL(search);
	FREE_HASHTABLE(Z_ARRVAL_P(replace));
    FREE_ZVAL(replace);
	//zval_ptr_dtor(&search);
	//zval_ptr_dtor(&replace); 
	RETURN_ZVAL(&retval,0,0);

}
PHP_METHOD(sec,xss_hash)
{
	zval *_xss_hash,*retval,*params[2],func;
	_xss_hash=zend_read_property(sec_ce,getThis(),"_xss_hash",sizeof("_xss_hash")-1,0);
	if(Z_TYPE_P(_xss_hash) == IS_NULL){
		MAKE_STD_ZVAL(retval);
		ZVAL_STRING(&func,"mt_rand",0);
		call_user_function(EG(function_table),NULL,&func,retval,0,params);
		MAKE_STD_ZVAL(params[0]);
	    MAKE_STD_ZVAL(params[1]);	
		ZVAL_ZVAL(params[0],retval,0,0);
		ZVAL_BOOL(params[1],1);
		ZVAL_STRING(&func,"uniqid",0);
		call_user_function(EG(function_table),NULL,&func,retval,2,params);
		ZVAL_STRING(&func,"md5",0);
		ZVAL_ZVAL(params[0],retval,0,0);
		call_user_function(EG(function_table),NULL,&func,retval,1,params);
		zend_update_property_string(sec_ce,getThis(),"_xss_hash",sizeof("_xss_hash")-1,Z_STRVAL_P(retval));
		RETURN_ZVAL(retval,0,0);
		zval_ptr_dtor(&retval);
		FREE_ZVAL(params[0]);
		FREE_ZVAL(params[1]);
	}else{
	//要深拷贝，不能直接赋值string的地址，因为如果在脚本调用后没有赋值到变量，即引用数减1后为0，立刻就会释放内存，会影响对象中的属性（因为地址相同）
	//RETURN_ZVAL会对ref_count进行减1
	RETURN_ZVAL(_xss_hash,1,0);
	}
}
PHP_METHOD(sec,entity_decode)
{
	zval func,*params[3],*retval,*str,old_str,*matches,**matches_0,*replace=NULL,**ele_value,*keys,*values;
	static zval *_entities=NULL;
	char *input;
    int input_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS(),"s",&input,&input_len)==FAILURE){
        return;
    }
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	MAKE_STD_ZVAL(retval);
	ZVAL_STRING(&func,"strpos",0);
	ZVAL_STRING(params[0],input,0);
	ZVAL_STRING(params[1],"&",0);
	MAKE_STD_ZVAL(str);
	ZVAL_ZVAL(str,params[0],1,0);
	call_user_function(EG(function_table),NULL,&func,retval,2,params);
	if(Z_TYPE_P(retval)==IS_BOOL&&(Z_LVAL_P(retval)==0)){
		FREE_ZVAL(params[1]);
		RETURN_ZVAL(params[0],0,0);	
		FREE_ZVAL(params[0]);
		zval_ptr_dtor(&retval);
	}else{
		zval_dtor(retval);
		if(_entities==NULL){
			MAKE_STD_ZVAL(_entities);
			ZVAL_STRING(&func,"get_html_translation_table",0);
			//HTML_ENTITIES is equal to 1
			//ENT_COMPAT | ENT_HTML5 is equal to 50
			ZVAL_LONG(params[0],1);	
			ZVAL_LONG(params[1],50);	
			ZVAL_STRING(params[2],"utf-8",0);
			call_user_function(EG(function_table),NULL,&func,_entities,3,params);
			ZVAL_STRING(&func,"array_map",0);
			ZVAL_STRING(params[0],"strtolower",0);
			ZVAL_ZVAL(params[1],_entities,0,0);
			call_user_function(EG(function_table),NULL,&func,_entities,2,params);
			zval_dtor(params[1]);
		}
		ZVAL_NULL(&old_str);
		MAKE_STD_ZVAL(matches);
		MAKE_STD_ZVAL(replace);
		MAKE_STD_ZVAL(keys);
		MAKE_STD_ZVAL(values);
		ZVAL_NULL(replace);
		do{
			if(Z_TYPE_P(&old_str) != IS_NULL){
                zval_dtor(&old_str);
            } 	
			ZVAL_ZVAL(&old_str,str,0,0);
			ZVAL_STRING(&func,"preg_match_all",0);
			ZVAL_STRING(params[0],"/&[a-z]{2,}(?![a-z;])/i",0);
			ZVAL_ZVAL(params[1],str,0,0);
			ZVAL_NULL(params[2]);
			call_user_function(EG(function_table),NULL,&func,retval,3,params);
			ZVAL_ZVAL(matches,params[2],0,0);
			if(Z_LVAL_P(retval)){
				zval_dtor(retval);
				zend_hash_index_find(Z_ARRVAL_P(matches),0,(void **)&matches_0);			
				ZVAL_STRING(&func,"array_map",0);
				ZVAL_STRING(params[0],"strtolower",0);
				ZVAL_ZVAL(params[1],*matches_0,0,0);
				call_user_function(EG(function_table),NULL,&func,retval,2,params);
				zval_dtor(matches);
				ZVAL_STRING(&func,"array_unique",0);
				ZVAL_ZVAL(params[0],retval,0,0);
				call_user_function(EG(function_table),NULL,&func,matches,1,params);
				zval_dtor(retval);
				array_init(replace);
				for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(matches));
					zend_hash_has_more_elements(Z_ARRVAL_P(matches)) == SUCCESS;
					zend_hash_move_forward(Z_ARRVAL_P(matches))) {
					zval *tmp_copy;
					char *tmp_str;
					if (zend_hash_get_current_data(Z_ARRVAL_P(matches), (void**)&ele_value) == FAILURE) {
						/* Should never actually fail since the key is known to exist.*/
						continue;
					}
					tmp_copy=*ele_value;
					tmp_str=(char *)malloc(strlen(Z_STRVAL_P(tmp_copy))+strlen(";")+1);
				    strcpy(tmp_str,Z_STRVAL_P(tmp_copy));
				    strcat(tmp_str,";");
				    ZVAL_STRING(params[0],tmp_str,1);
				    free(tmp_str);
				    ZVAL_ZVAL(params[1],_entities,0,0);
				    ZVAL_BOOL(params[2],1);
					ZVAL_STRING(&func,"array_search",0);
					call_user_function(EG(function_table),NULL,&func,retval,3,params);	
					zval_dtor(params[0]);
					if(Z_TYPE_P(retval)!=IS_BOOL&&Z_LVAL_P(retval)!=0){
						add_assoc_string(replace,Z_STRVAL_P(tmp_copy),Z_STRVAL_P(retval),1);
					}
					zval_dtor(retval);	
				}
					ZVAL_STRING(&func,"array_keys",0);
					ZVAL_ZVAL(params[0],replace,0,0);
					call_user_function(EG(function_table),NULL,&func,keys,1,params);
					ZVAL_STRING(&func,"array_values",0);
					ZVAL_ZVAL(params[0],replace,0,0);
					call_user_function(EG(function_table),NULL,&func,values,1,params);
					ZVAL_STRING(&func,"str_replace",0);
					ZVAL_ZVAL(params[0],keys,0,0);
					ZVAL_ZVAL(params[1],values,0,0);
					ZVAL_ZVAL(params[2],str,0,0);
					call_user_function(EG(function_table),NULL,&func,str,3,params);
					zval_dtor(replace);
					zval_dtor(keys);
					zval_dtor(values);
					zval_dtor(matches);

			}
			ZVAL_STRING(&func,"preg_replace",0);
			ZVAL_STRING(params[0],"/(&#(?:x0*[0-9a-f]{2,5}(?![0-9a-f;])|(?:0*\\d{2,4}(?![0-9;]))))/iS",0);
			ZVAL_STRING(params[1],"$1;",0);
			ZVAL_ZVAL(params[2],str,0,0);
			call_user_function(EG(function_table),NULL,&func,retval,3,params);
			if(Z_STRVAL(old_str)!=Z_STRVAL_P(str)){
				zval_dtor(str);	
			}
			ZVAL_STRING(&func,"html_entity_decode",0);
			ZVAL_ZVAL(params[0],retval,0,0);
			ZVAL_LONG(params[1],50);
			ZVAL_STRING(params[2],"UTF-8",0);
			call_user_function(EG(function_table),NULL,&func,str,3,params);
			zval_dtor(params[0]);
		}while(strcmp(Z_STRVAL(old_str),Z_STRVAL_P(str))!=0);	
		if(Z_TYPE_P(&old_str) != IS_NULL){
                zval_dtor(&old_str);
        }
	    FREE_ZVAL(replace);
		FREE_ZVAL(params[0]);
		FREE_ZVAL(params[1]);
		FREE_ZVAL(params[2]);
		FREE_ZVAL(retval);
		//跟类的属性同理，不要直接赋值，因为会导致在脚本阶段输出后-1就释放了
        RETURN_ZVAL(str,1,0);
        zval_ptr_dtor(&str);
		FREE_ZVAL(matches);
        FREE_ZVAL(keys);
        FREE_ZVAL(values);
        //zval_ptr_dtor(&retval);
	}
}
PHP_METHOD(sec,_decode_entity)
{
	zval func,*params[3],*match,**input,*retval;
	char *str;
	if(zend_parse_parameters(ZEND_NUM_ARGS(),"a",&match)==FAILURE){
        return;
    }
	zval *object=getThis();
	zval tmp_replace;
	INIT_ZVAL(tmp_replace);
    zend_hash_index_find(Z_ARRVAL_P(match),0,(void **)&input);
    MAKE_STD_ZVAL(params[0]);
    MAKE_STD_ZVAL(params[1]);
    MAKE_STD_ZVAL(params[2]);
    MAKE_STD_ZVAL(retval);
	ZVAL_STRING(&func,"xss_hash",0);
	call_user_function(EG(function_table),&object,&func,retval,0,params);	
	str=(char *)malloc(strlen(Z_STRVAL_P(retval))+strlen("\\1=\\2")+1);
	ZVAL_ZVAL(&tmp_replace,retval,0,0);
	strcpy(str,Z_STRVAL_P(retval));
	strcat(str,"\\1=\\2");
	ZVAL_STRING(retval,str,1);
	free(str);
	ZVAL_STRING(&func,"preg_replace",0);
	ZVAL_STRING(params[0],"|\\&([a-z\\_0-9\\-]+)\\=([a-z\\_0-9\\-/]+)|i",0);
    ZVAL_ZVAL(params[1],retval,0,0);
    ZVAL_ZVAL(params[2],*input,0,0);
	ZVAL_STRING(&func,"entity_decode",0);
	call_user_function(EG(function_table),&object,&func,retval,3,params);
	zval_dtor(params[1]);
	ZVAL_ZVAL(params[0],&tmp_replace,0,0);
	ZVAL_STRING(params[1],"&",0);
	ZVAL_ZVAL(params[2],retval,0,0);
	ZVAL_STRING(&func,"str_replace",0);
	call_user_function(EG(function_table),NULL,&func,retval,3,params);
	RETURN_ZVAL(retval,0,0);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[2]);
}
PHP_METHOD(sec, xss_clean )
{
    char *str;
    int str_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS(),"s",&str,&str_len)==FAILURE){
        return;
    }
    zval *new_str,func,*params[3],retval,old_str,*param_arr;
    zval *object=getThis();
    new_str=remove_invisible_characters(str,str_len,true);
    //RETURN_ZVAL(new_str,0,0);
    ZVAL_STRING(&func,"stripos",0);
	MAKE_STD_ZVAL(params[0]);
    MAKE_STD_ZVAL(params[1]);
    MAKE_STD_ZVAL(params[2]);
    MAKE_STD_ZVAL(param_arr);
	array_init(param_arr);
	//can not be covered,because every time add in the used index will free the old one,will cause segment fault
	add_index_zval(param_arr,0,object);
    //MAKE_STD_ZVAL(old_str);
    ZVAL_ZVAL(params[0],new_str,0,0);
    ZVAL_STRING(params[1],"%",0);
    call_user_function(EG(function_table),NULL,&func,&retval,2,params);
    if(Z_TYPE(retval)!=IS_BOOL||(Z_LVAL(retval)!=0)){
		//RETURN_ZVAL(&retval,1,0);	
		ZVAL_NULL(&old_str);
	    //multi add must be dup,because next cover will destory the old one use zval_ptr_dtor	
		add_index_string(param_arr,1,"_urldecodespaces",1);	
		do{
			//free old_str if it is copy from new_str
			if(Z_TYPE_P(&old_str) != IS_NULL){
				zval_dtor(&old_str);
			}
    		//array_init(param_arr);
			ZVAL_ZVAL(&old_str,new_str,0,0);
			ZVAL_ZVAL(params[0],new_str,0,0);
    		ZVAL_STRING(&func,"rawurldecode",0);
			call_user_function(EG(function_table),NULL,&func,new_str,1,params);
			//ZVAL_ZVAL(new_str,&retval,0,0);
			ZVAL_STRING(params[0],"#%(?:\\s*[0-9a-f]){2,}#i",0);
			ZVAL_ZVAL(params[1],param_arr,0,0);
			ZVAL_ZVAL(params[2],new_str,0,0);
			ZVAL_STRING(&func,"preg_replace_callback",0);
			call_user_function(EG(function_table),NULL,&func,&retval,3,params);
			zval_dtor(new_str);
			ZVAL_ZVAL(new_str,&retval,0,0);
		}while(strcmp(Z_STRVAL(old_str),Z_STRVAL_P(new_str))!=0);
		zval_dtor(&old_str);
		//RETURN_ZVAL(new_str,0,0);
	}
    ZVAL_STRING(&func,"preg_replace_callback",0);
	ZVAL_STRING(params[0],"/[^a-z0-9>]+[a-z0-9]+=([\\'\"]).*?\\1/si",0);
	add_index_string(param_arr,1,"_convert_attribute",1);	
	ZVAL_ZVAL(params[1],param_arr,0,0);
	ZVAL_ZVAL(params[2],new_str,0,0);
	call_user_function(EG(function_table),NULL,&func,&retval,3,params);
	zval_dtor(new_str);
	ZVAL_ZVAL(new_str,&retval,0,0);
	//RETURN_ZVAL(new_str,0,0);
	ZVAL_STRING(params[0],"/<\\w+.*/si",0);
	add_index_string(param_arr,1,"_decode_entity",1);	
    ZVAL_ZVAL(params[1],param_arr,0,0);
    ZVAL_ZVAL(params[2],new_str,0,0);
	call_user_function(EG(function_table),NULL,&func,&retval,3,params);
	zval_dtor(new_str);
	ZVAL_ZVAL(new_str,&retval,0,0);
	
	RETURN_ZVAL(new_str,0,0);
	//free
	add_index_string(param_arr,1,"",0);//添加两个空的字符串，为了释放原先在里面的内存
	add_index_string(param_arr,2,"",0);//添加两个空的字符串，为了释放原先在里面的内存
	FREE_HASHTABLE(Z_ARRVAL_P(param_arr));
    FREE_ZVAL(param_arr);
	FREE_ZVAL(params[0]);
    FREE_ZVAL(params[1]);
    FREE_ZVAL(params[2]);

}
static zend_function_entry filter_method[]={
    PHP_ME(sec, _urldecodespaces,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, _convert_attribute,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, xss_hash,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, entity_decode,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, _decode_entity,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, xss_clean,  NULL,   ZEND_ACC_PUBLIC)
    {NULL,NULL,NULL}

};
PHP_MINIT_FUNCTION(sec)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce,"sec",filter_method);
    sec_ce=zend_register_internal_class(&ce);
	zend_declare_property_null(sec_ce, "_xss_hash", strlen("_xss_hash"), ZEND_ACC_PROTECTED);
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

