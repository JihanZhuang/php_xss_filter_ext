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
	//tmp_replace is xss_hash
	ZVAL_ZVAL(&tmp_replace,retval,0,0);
	strcpy(str,Z_STRVAL_P(retval));
	strcat(str,"\\1=\\2");
	ZVAL_STRING(retval,str,1);
	free(str);
	ZVAL_STRING(&func,"preg_replace",0);
	ZVAL_STRING(params[0],"|\\&([a-z\\_0-9\\-]+)\\=([a-z\\_0-9\\-/]+)|i",0);
    ZVAL_ZVAL(params[1],retval,0,0);
    ZVAL_ZVAL(params[2],*input,0,0);
	call_user_function(EG(function_table),NULL,&func,retval,3,params);
	zval_dtor(params[1]);
	ZVAL_STRING(&func,"entity_decode",0);
	ZVAL_ZVAL(params[0],retval,0,0);
	call_user_function(EG(function_table),&object,&func,retval,1,params);
	zval_dtor(params[0]);
	ZVAL_ZVAL(params[0],&tmp_replace,0,0);
	ZVAL_STRING(params[1],"&",0);
	ZVAL_ZVAL(params[2],retval,0,0);
	ZVAL_STRING(&func,"str_replace",0);
	call_user_function(EG(function_table),NULL,&func,retval,3,params);
	RETURN_ZVAL(retval,0,0);
	zval_ptr_dtor(&params[0]);
	zval_ptr_dtor(&params[2]);
}
PHP_METHOD(sec,_do_never_allowed)
{
	zval func,*params[3],*str,*retval,*keys,*_never_allowed_str,*_never_allowed_regex,**ele_value;
    if(zend_parse_parameters(ZEND_NUM_ARGS(),"z",&str)==FAILURE){
        return;
    }	
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	MAKE_STD_ZVAL(keys);
	MAKE_STD_ZVAL(retval);
	_never_allowed_str=zend_read_property(sec_ce,getThis(),"_never_allowed_str",strlen("_never_allowed_str"),0);	
	_never_allowed_regex=zend_read_property(sec_ce,getThis(),"_never_allowed_regex",strlen("_never_allowed_regex"),0);	
	ZVAL_STRING(&func,"array_keys",0);
	ZVAL_ZVAL(params[0],_never_allowed_str,0,0);
	call_user_function(EG(function_table),NULL,&func,keys,1,params);
	ZVAL_STRING(&func,"str_replace",0);
	ZVAL_ZVAL(params[0],keys,0,0);
	ZVAL_ZVAL(params[1],_never_allowed_str,0,0);
	ZVAL_ZVAL(params[2],str,0,0);
	call_user_function(EG(function_table),NULL,&func,retval,3,params);	
	zval_ptr_dtor(&keys);
	for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(_never_allowed_regex));
                    zend_hash_has_more_elements(Z_ARRVAL_P(_never_allowed_regex)) == SUCCESS;
                    zend_hash_move_forward(Z_ARRVAL_P(_never_allowed_regex))) {
                    zval *tmp_copy;
					char *tmp_str;
                    if (zend_hash_get_current_data(Z_ARRVAL_P(_never_allowed_regex), (void**)&ele_value) == FAILURE) {
                        /* Should never actually fail since the key is known to exist.*/
                        continue;
                    }
					tmp_copy=*ele_value;
                    tmp_str=(char *)malloc(strlen(Z_STRVAL_P(tmp_copy))+strlen("#")+strlen("#is")+1);
					strcpy(tmp_str,"#");
                    strcat(tmp_str,Z_STRVAL_P(tmp_copy));
                    strcat(tmp_str,"#is");
                    ZVAL_STRING(params[0],tmp_str,1);
                    free(tmp_str);
					ZVAL_STRING(&func,"preg_replace",0);
                    ZVAL_STRING(params[1],"[removed]",0);
                    ZVAL_ZVAL(params[2],retval,0,0);
					call_user_function(EG(function_table),NULL,&func,retval,3,params);
					zval_dtor(params[2]);
    }		
	RETURN_ZVAL(retval,0,0);
	FREE_ZVAL(params[0]);
	FREE_ZVAL(params[1]);
	FREE_ZVAL(params[2]);
	FREE_ZVAL(retval);
	
}
PHP_METHOD(sec,_compact_exploded_words)
{
	zval *matches,**matches_0,**matches_1,*params[3],func,retval;
	char *str;
	if(zend_parse_parameters(ZEND_NUM_ARGS(),"a",&matches)==FAILURE){
        return;
    }
	zend_hash_index_find(Z_ARRVAL_P(matches),0,(void **)&matches_0);
	zend_hash_index_find(Z_ARRVAL_P(matches),1,(void **)&matches_1);
	MAKE_STD_ZVAL(params[0]);
	MAKE_STD_ZVAL(params[1]);
	MAKE_STD_ZVAL(params[2]);
	ZVAL_STRING(params[0],"/\\s+/s",0);
	ZVAL_STRING(params[1],"",0);
	ZVAL_ZVAL(params[2],*matches_0,0,0);
	ZVAL_STRING(&func,"preg_replace",0);
	call_user_function(EG(function_table),NULL,&func,&retval,3,params);
	str=(char *)malloc(strlen(Z_STRVAL(retval))+strlen(Z_STRVAL_P(*matches_1))+1);
	strcpy(str,Z_STRVAL(retval));
	strcat(str,Z_STRVAL_P(*matches_1));
	RETURN_STRING(str,1);
	free(str);
	FREE_ZVAL(params[0]);
	FREE_ZVAL(params[1]);
	FREE_ZVAL(params[2]);
}
PHP_METHOD(sec,_sanitize_naughty_html)
{
	zval *naughty_tags,*evil_attributes,func,*matches,**input,**input_index,retval,*params[3];
	if(zend_parse_parameters(ZEND_NUM_ARGS(),"z",&matches)==FAILURE){
        return;
    }
	zend_hash_find(Z_ARRVAL_P(matches),"closeTag",9,(void**) &input);
	ZVAL_STRING(&func,"empty",0);
	MAKE_STD_ZVAL(params[0]);
	ZVAL_ZVAL(params[0],*input,0,0);
	INIT_ZVAL(retval);
	call_user_function(EG(function_table),NULL,&func,&retval,1,params);
	if(Z_TYPE(retval)==IS_BOOL&&Z_LVAL(retval)==1){
		char *tmp_str=NULL;
		zend_hash_index_find(Z_ARRVAL_P(matches),1,(void **)&input_index);
		tmp_str=(char *)malloc(strlen(Z_STRVAL_P(*input_index))+strlen("&lt;")+1);
		strcpy(tmp_str,"&lt;");	
		strcat(tmp_str,Z_STRVAL_P(*input_index));
		RETURN_STRING(tmp_str,1);
		free(tmp_str);
		FREE_ZVAL(params[0]);	
		return;
	}	
	naughty_tags=zend_read_property(sec_ce,getThis(),"_naughty_tags",strlen("_naughty_tags"),0);
	evil_attributes=zend_read_property(sec_ce,getThis(),"_evil_attributes",strlen("_evil_attributes"),0);
	
}
PHP_METHOD(sec, xss_clean )
{
    char *str;
    int str_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS(),"s",&str,&str_len)==FAILURE){
        return;
    }
    zval *new_str,func,*params[3],retval,old_str,*param_arr,*_words,**ele_value;
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
	zval_ptr_dtor(&new_str);
	new_str=remove_invisible_characters(Z_STRVAL(retval),Z_STRLEN(retval),true);	
	zval_dtor(&retval);
	//entity_decode
	//RETURN_ZVAL(new_str,0,0);	
    ZVAL_STRING(&func,"str_replace",0);
    ZVAL_STRING(params[0],"\\t",0);
    ZVAL_STRING(params[1]," ",0);
    ZVAL_ZVAL(params[2],new_str,0,0);
	call_user_function(EG(function_table),NULL,&func,&retval,3,params);
	zval_dtor(new_str);
	ZVAL_ZVAL(new_str,&retval,0,0);
	ZVAL_ZVAL(params[0],new_str,0,0);
	ZVAL_STRING(&func,"_do_never_allowed",0);
	call_user_function(EG(function_table),&object,&func,&retval,1,params);
	zval_dtor(new_str);
	ZVAL_ZVAL(new_str,&retval,0,0);
	ZVAL_STRING(&func,"str_replace",0);
	array_init(params[0]);
	array_init(params[1]);
	add_next_index_string(params[0],"<?",0);
	add_next_index_string(params[0],"?>",0);
	add_next_index_string(params[1],"&lt;?",0);
	add_next_index_string(params[1],"?&gt;",0);
	ZVAL_ZVAL(params[2],new_str,0,0);
	call_user_function(EG(function_table),NULL,&func,&retval,3,params);
	zval_dtor(new_str);
	ZVAL_ZVAL(new_str,&retval,0,0);
	_words=zend_read_property(sec_ce,getThis(),"_words",strlen("_words"),0);	
	for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(_words));
                    zend_hash_has_more_elements(Z_ARRVAL_P(_words)) == SUCCESS;
                    zend_hash_move_forward(Z_ARRVAL_P(_words))) {
                    zval *tmp_copy;
                    char *tmp_str;
                    if (zend_hash_get_current_data(Z_ARRVAL_P(_words), (void**)&ele_value) == FAILURE) {
                        /* Should never actually fail since the key is known to exist.*/
                        continue;
                    }
                    tmp_copy=*ele_value;
                    ZVAL_ZVAL(params[0],tmp_copy,0,0);
					ZVAL_STRING(&func,"str_split",0);
					call_user_function(EG(function_table),NULL,&func,&retval,1,params);
                    ZVAL_STRING(params[0],"\\s*",0);
                    ZVAL_ZVAL(params[1],&retval,0,0);
					ZVAL_STRING(&func,"implode",0);
					call_user_function(EG(function_table),NULL,&func,&retval,2,params);
					zval_dtor(params[1]);
                    tmp_str=(char *)malloc(strlen(Z_STRVAL(retval))+strlen("\\s*")+1);
                    strcpy(tmp_str,Z_STRVAL(retval));
                    strcat(tmp_str,"\\s*");
                    ZVAL_STRING(params[0],tmp_str,1);
                    free(tmp_str);
                    ZVAL_STRING(&func,"substr",0);
                    ZVAL_LONG(params[1],0);
                    ZVAL_LONG(params[2],-3);
                    call_user_function(EG(function_table),NULL,&func,&retval,3,params);
                    zval_dtor(params[0]);
                    tmp_str=(char *)malloc(strlen(Z_STRVAL(retval))+strlen("#(")+strlen(")(\\W)#is")+1);
					strcpy(tmp_str,"#(");
                    strcat(tmp_str,Z_STRVAL(retval));
					zval_dtor(&retval);
                    strcat(tmp_str,")(\\W)#is");
                    ZVAL_STRING(params[0],tmp_str,1);
                    free(tmp_str);
					add_index_string(param_arr,1,"_compact_exploded_words",1);
                    ZVAL_ZVAL(params[1],param_arr,0,0);
                    ZVAL_ZVAL(params[2],new_str,0,0);
                    ZVAL_STRING(&func,"preg_replace_callback",0);
					call_user_function(EG(function_table),NULL,&func,&retval,3,params);
					zval_dtor(params[0]);
					zval_dtor(params[2]);
					ZVAL_ZVAL(new_str,&retval,0,0);
    }
	//RETURN_ZVAL(new_str,0,0);	
	//free
	add_index_string(param_arr,1,"",0);//添加两个空的字符串，为了释放原先在里面的内存
	add_index_string(param_arr,2,"",0);//添加两个空的字符串，为了释放原先在里面的内存
	FREE_HASHTABLE(Z_ARRVAL_P(param_arr));
    FREE_ZVAL(param_arr);
	FREE_ZVAL(params[0]);
    FREE_ZVAL(params[1]);
    FREE_ZVAL(params[2]);

}
PHP_METHOD(sec,__construct)
{
	zval *_never_allowed_str,*_never_allowed_regex,*_words,*_naughty_tags,*_evil_attributes;
	MAKE_STD_ZVAL(_never_allowed_str);
	array_init(_never_allowed_str);	
	add_assoc_string(_never_allowed_str,"document.cookie","[removed]",1);	
	add_assoc_string(_never_allowed_str,"document.write","[removed]",1);	
	add_assoc_string(_never_allowed_str,".parentNode","[removed]",1);	
	add_assoc_string(_never_allowed_str,".innerHTML","[removed]",1);	
	add_assoc_string(_never_allowed_str,"-moz-binding","[removed]",1);	
	add_assoc_string(_never_allowed_str,"<!--","&lt;!--",1);	
	add_assoc_string(_never_allowed_str,"-->","--&gt;",1);	
	add_assoc_string(_never_allowed_str,"<![CDATA[","&lt;![CDATA[",1);	
	add_assoc_string(_never_allowed_str,"<comment>","&lt;comment&gt;",1);	
	add_assoc_string(_never_allowed_str,"<%","&lt;&#37;",1);	
	zend_update_property(sec_ce,getThis(),"_never_allowed_str",strlen("_never_allowed_str"),_never_allowed_str);
	MAKE_STD_ZVAL(_never_allowed_regex);
	array_init(_never_allowed_regex);	
    add_next_index_string(_never_allowed_regex,"javascript\\s*:",1); 
    add_next_index_string(_never_allowed_regex,"(document|(document\\.)?window)\\.(location|on\\w*)",1);  
    add_next_index_string(_never_allowed_regex,"expression\\s*(\\(|&\\#40;)",1); 
    add_next_index_string(_never_allowed_regex,"vbscript\\s*:",1);  
    add_next_index_string(_never_allowed_regex,"wscript\\s*:",1);    
    add_next_index_string(_never_allowed_regex,"jscript\\s*:",1);    
    add_next_index_string(_never_allowed_regex,"vbs\\s*:",1); 
    add_next_index_string(_never_allowed_regex,"Redirect\\s+30\\d",1);   
    add_next_index_string(_never_allowed_regex,"([\"'])?data\\s*:[^\\1]*?base64[^\\1]*?,[^\\1]*?\\1?",1);   
	zend_update_property(sec_ce,getThis(),"_never_allowed_regex",strlen("_never_allowed_regex"),_never_allowed_regex);
	MAKE_STD_ZVAL(_words);
	array_init(_words);	
    add_next_index_string(_words,"javascript",1); 
    add_next_index_string(_words,"expression",1);  
    add_next_index_string(_words,"vbscript",1); 
    add_next_index_string(_words,"jscript",1);  
    add_next_index_string(_words,"wscript",1);        
    add_next_index_string(_words,"vbs",1);        
    add_next_index_string(_words,"script",1); 
    add_next_index_string(_words,"base64",1);   
    add_next_index_string(_words,"applet",1);
    add_next_index_string(_words,"alert",1);
    add_next_index_string(_words,"document",1);
    add_next_index_string(_words,"write",1);
    add_next_index_string(_words,"cookie",1);
    add_next_index_string(_words,"window",1);
    add_next_index_string(_words,"confirm",1);
    add_next_index_string(_words,"prompt",1);
    add_next_index_string(_words,"eval",1);
    zend_update_property(sec_ce,getThis(),"_words",strlen("_words"),_words);
    MAKE_STD_ZVAL(_naughty_tags);
    array_init(_naughty_tags); 
	add_next_index_string(_naughty_tags,"alert",1);
	add_next_index_string(_naughty_tags,"area",1);
	add_next_index_string(_naughty_tags,"prompt",1);
	add_next_index_string(_naughty_tags,"confirm",1);
	add_next_index_string(_naughty_tags,"applet",1);
	add_next_index_string(_naughty_tags,"audio",1);
	add_next_index_string(_naughty_tags,"basefont",1);
	add_next_index_string(_naughty_tags,"base",1);
	add_next_index_string(_naughty_tags,"behavior",1);
	add_next_index_string(_naughty_tags,"bgsound",1);
	add_next_index_string(_naughty_tags,"blink",1);
	add_next_index_string(_naughty_tags,"body",1);
	add_next_index_string(_naughty_tags,"embed",1);
	add_next_index_string(_naughty_tags,"expression",1);
	add_next_index_string(_naughty_tags,"form",1);
	add_next_index_string(_naughty_tags,"frameset",1);
	add_next_index_string(_naughty_tags,"frame",1);
	add_next_index_string(_naughty_tags,"head",1);
	add_next_index_string(_naughty_tags,"html",1);
	add_next_index_string(_naughty_tags,"ilayer",1);
	add_next_index_string(_naughty_tags,"iframe",1);
	add_next_index_string(_naughty_tags,"input",1);
	add_next_index_string(_naughty_tags,"button",1);
	add_next_index_string(_naughty_tags,"select",1);
	add_next_index_string(_naughty_tags,"isindex",1);
	add_next_index_string(_naughty_tags,"layer",1);
	add_next_index_string(_naughty_tags,"link",1);
	add_next_index_string(_naughty_tags,"meta",1);
	add_next_index_string(_naughty_tags,"keygen",1);
	add_next_index_string(_naughty_tags,"object",1);
	add_next_index_string(_naughty_tags,"plaintext",1);
	add_next_index_string(_naughty_tags,"style",1);
	add_next_index_string(_naughty_tags,"script",1);
	add_next_index_string(_naughty_tags,"textarea",1);
	add_next_index_string(_naughty_tags,"title",1);
	add_next_index_string(_naughty_tags,"math",1);
	add_next_index_string(_naughty_tags,"video",1);
	add_next_index_string(_naughty_tags,"svg",1);
	add_next_index_string(_naughty_tags,"xml",1);
	add_next_index_string(_naughty_tags,"xss",1);
    zend_update_property(sec_ce,getThis(),"_naughty_tags",strlen("_naughty_tags"),_naughty_tags);
    MAKE_STD_ZVAL(_evil_attributes);
    array_init(_evil_attributes); 
	add_next_index_string(_evil_attributes,"on\\w+",1);
	add_next_index_string(_evil_attributes,"style",1);
	add_next_index_string(_evil_attributes,"xmlns",1);
	add_next_index_string(_evil_attributes,"formaction",1);
	add_next_index_string(_evil_attributes,"form",1);
	add_next_index_string(_evil_attributes,"xlink:href",1);
	add_next_index_string(_evil_attributes,"FSCommand",1);
	add_next_index_string(_evil_attributes,"seekSegmentTime",1);
    zend_update_property(sec_ce,getThis(),"_evil_attributes",strlen("_evil_attributes"),_evil_attributes);

}
static zend_function_entry filter_method[]={
    PHP_ME(sec, _urldecodespaces,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, _convert_attribute,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, xss_hash,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, entity_decode,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, _decode_entity,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, _do_never_allowed,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, _compact_exploded_words,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, _sanitize_naughty_html,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, xss_clean,  NULL,   ZEND_ACC_PUBLIC)
    PHP_ME(sec, __construct,  NULL,   ZEND_ACC_PUBLIC)
    {NULL,NULL,NULL}

};
PHP_MINIT_FUNCTION(sec)
{
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce,"sec",filter_method);
    sec_ce=zend_register_internal_class(&ce);
	zend_declare_property_null(sec_ce, "_xss_hash", strlen("_xss_hash"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(sec_ce, "_never_allowed_str", strlen("_never_allowed_str"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(sec_ce, "_never_allowed_regex", strlen("_never_allowed_regex"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(sec_ce, "_words", strlen("_words"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(sec_ce, "_naughty_tags", strlen("_naughty_tags"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(sec_ce, "_evil_attributes", strlen("_evil_attributes"), ZEND_ACC_PUBLIC);
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

