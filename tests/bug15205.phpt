--TEST--
Check for simplate comment
--FILE--
<?php
if(!extension_loaded('simplate')) { dl('simplate.so'); }

$simplate = new Simplate();
$simplate->force_compile = true;
$simplate->assign("hoge","foo");
$simplate->display("bug15205.tpl");

?>
--EXPECT--
foofoo
