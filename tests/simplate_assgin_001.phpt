--TEST--
Check for Simplate assign
--FILE--
<?php
$simplate = new Simplate();
$simplate->assign('key1','val1');
echo $simplate->_tpl_vars['key1'];
?>
--EXPECT--
val1
