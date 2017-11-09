<?php
$a=new sec();
for($i=0;$i<10;$i++){
$c=$a->xss_clean("<a class=\"navbar-link\" href=\"/&lt;a class=&quot;navbar-link&quot; href=&quot;/zh_TW&quot; title=&quot;在線字符串轉換工具&quot;&gt;字符串轉換&lt;/a&gt;zh_TW\" title=\"在線字符串轉換工具\">字符串轉換</a>");
var_dump($c);
}
var_dump($a->_never_allowed_str);
var_dump($a->_never_allowed_regex);
