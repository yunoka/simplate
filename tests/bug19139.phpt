--TEST--
Check for recursive creating directory (win)
--FILE--
<?php
if(!extension_loaded('simplate')) { dl('simplate.so'); }

$simplate = new Simplate();
$simplate->force_compile = true;
$simplate->assign("message","success!!");
$simplate->display("bug/19139/bug19139.tpl");

?>
--EXPECT--
<html>
<body>
template/bug/19139/bug19139.tpl
<b>success!!</b>
</body>
</html>
