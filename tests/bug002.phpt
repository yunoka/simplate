--TEST--
Check for simplate variable parser with function in if-statement.
--FILE--
<?php
if (!extension_loaded('simplate')) { dl('simplate.so'); }

define('OPTION', 'opt');
function func($a, $b){
  return 1;
}
$ary = array('foo' => 'bar');
$obj = new stdClass();
$obj->foo = "I'm stdClass's foo.";

$oSimplate=new Simplate();
$oSimplate->force_compile = true;
$oSimplate->left_delimiter = "{";
$oSimplate->right_delimiter = "}";
$oSimplate->assign('foo', 'foo');
$oSimplate->assign('ary', $ary);
$oSimplate->assign('obj', $obj);
$oSimplate->display("bug002.tpl");

?>
--EXPECT--
var:fooary:barobj:I'm stdClass's foo.
