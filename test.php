<?php
$a=new sec();
for($i=0;$i<10;$i++){
$c=$a->xss_clean("<a ja va scri pt class=\"navbar-link\" href=\"/&lt;a class=&quot;navbar-link&quot; href=&quot;/zh_TW&quot; title=&quot;在線字符串轉換工具&quot;&gt;字符串轉換&lt;/a&gt;zh_TW\" title=\"在線字符串轉換工具\">字符串轉換</a>");
var_dump($c);
echo $a->_do_never_allowed("safdasfsadfsadfsafs  document.cookie  <![CDATA[   document.window.location");
}
var_dump($a->_words);
echo $a->_compact_exploded_words(array('sdf sad sadf asdf asd ','123123123'));
