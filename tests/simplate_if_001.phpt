--TEST--
Check for Simplate If-Statement
--FILE--
<?php

class Klass{
  function hoge($str="test"){
    return "Klass::hoge($str)\n";
  }
  function bar(){
    return "Klass::bar()\n";
  }
}

$simplate = new Simplate();
$simplate->force_compile=true;

$bar = "bar\n";
$simplate->assign('bar',$bar);


// if $ary9[i]->sub[j]->bar
$ary9 = array();
for($i=0;$i<3;$i++){
  $o = new Klass();
  for($j=0;$j<2;$j++){
    $o->sub[] = (object)array('bar'=>$bar);
  }
  $ary9[] = $o;
}

$simplate->assign('ary9',$ary9);
$simplate->display("if_test01.tpl");


?>
--EXPECT--
// if $ary9[i]->sub[j]->bar
[found]bar
[found]bar
[found]bar
[found]bar
[found]bar
[found]bar



