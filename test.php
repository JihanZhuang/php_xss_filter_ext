<?php
$a=new sec();
for($i=0;$i<10;$i++){
echo $a->xss_clean("http://%   25%7f%77%77%2E%67%6F%6F%67%6C%65%2E%63%6F%6D<span style='color:#0\n066>00;'>&asfa=asf")."\n";
}
for($i=0;$i<1;$i++){
echo $a->xss_clean("http%3a%2f%2ftool.chinaz.com%2fTools%2furlencode.aspx")."\n";
}/*
$b=new sec();
for($i=0;$i<1000;$i++){
echo $b->xss_clean("http://%25%7f%77%77%2E%67%6F%6F%67%6C%65%2E%63%6F%6D<span style='color:#00\n66>00;'>&asfa=asf");
}
for($i=0;$i<1000;$i++){
echo $b->xss_clean("http%3a%2f%2ftool.chinaz.com%2fTo%      1aols%2furlencode.aspx")."\n";
}*/
