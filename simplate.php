<?php

if (!extension_loaded('simplate')) { dl('simplate.so'); }

define('START_TIME', microtime(true));
$smarty =& new simplate();
switch (@$_REQUEST['f']) {
case 2:
    $smarty->cache_lifetime = 600;
    $smarty->caching = true;
    break;
case 1:
    $smarty->force_compile = true;
    break;
default:
    $smarty->force_compile = false;
}
//$smarty->force_compile=true;

$hoge="aaaa";
$smarty->assign("hoge1", $hoge); // Seg fault??
$smarty->assign("myname", "Simplate"); // set string
$smarty->assign("greeting", "hello"); // set string
$smarty->assign("date", date("Y-m-d H:i:s")); // set string
$smarty->assign("ary", array("hoge1", "hoge2", "hoge3"));
$smarty->assign("ary2", array("foo1" => "bar1", "foo2" => "bar2", "foo3" => "bar3"));
$smarty->assign("custid", array("001-0001", "002-0003", "010-9999"));
$smarty->assign('contacts',
    array(
        array(
            'phone' => '1',
            'fax' => '2',
            'cell' => '3'),
        array(
            'phone' => '555-4444',
            'fax' => '555-3333',
            'cell' => '760-1234')
    ));
//$fetch=$smarty->fetch('fetch.tpl');
$smarty->assign("fetched_content", $smarty->fetch('fetch.tpl'));
//echo "fetch=$fetch";

$entry = array(
    array(
        "entry_id" => "hoge",
        "comments" => array(
            array("comment_id" => "comment1"),
            array("comment_id" => "comment2")
        )
    )
);
$smarty->assign("entry", $entry);
$smarty->register_prefilter('remove_comment');
$smarty->register_postfilter('convert_ad');

//$smarty->assign("fetched_content", $fetch);
//$smarty->assign("hoge2", $hoge); // Seg fault??

//$smarty->clear_assign("date"); // assign with null
//$smarty->assign("date", null);
//$smarty->clear_assign(array("date", "greeting"));
//var_dump($smarty);
//$smarty->fetch("sample.tpl");
$smarty->display("simplate.tpl");
//$smarty->clear_cache("simplate.tpl");

$end_time = sprintf("%01.04f", microtime(true) - START_TIME);
echo "<font color=red>$end_time</font>\n";

function remove_comment($source)
{
    return preg_replace("/<!--.*-->/U", "", $source);
}

function convert_ad($source)
{
  $before = array('/<\/body>/');
  $after = array('<script src="http://www.google-analytics.com/urchin.js" type="text/javascript"></script></body>');

  return preg_replace($before, $after, $source);
}
?>
