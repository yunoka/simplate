--TEST--
Check for simplate include with variables and force_compile=false
--FILE--
<?php
if(!extension_loaded('simplate')) { dl('simplate.so'); }

$simplate=new Simplate();
$simplate->assign("inc_file","fetch.tpl");
$simplate->assign("inc_file2","fetch");
$simplate->display("bug001.tpl");

?>
--EXPECT--
<h3>== fetch ==</h3>
fetched

<h3>== fetch ==</h3>
fetched

<h3>== fetch ==</h3>
fetched
