--TEST--
Check for Simplate foreach
--FILE--
<?php

$simplate = new Simplate();

// item
$arr = array(1000, 1001, 1002);
$simplate->assign('myArray', $arr);

// item & key
$arr = array(9 => 'Tennis', 3 => 'Swimming', 8 => 'Coding');
$simplate->assign('myArray2', $arr);

// item list
$items_list =
    array(
        23 => array('no' => 2456, 'label' => 'Salad'),
        96 => array('no' => 4889, 'label' => 'Cream')
    );
$simplate->assign('items', $items_list);

// foreach nest
$simplate->assign('contacts', array(
    array('phone' => '1',
        'fax' => '2',
        'cell' => '3'),
    array('phone' => '555-4444',
        'fax' => '555-3333',
        'cell' => '760-1234')
    ));

$simplate->display("foreach_01.tpl");

?>
--EXPECT--
== item ==
<ul>
    <li>1000</li>
    <li>1001</li>
    <li>1002</li>
</ul>

== item & key ==
<ul>
    <li>9: Tennis</li>
    <li>3: Swimming</li>
    <li>8: Coding</li>
</ul>

== item list ==
<ul>
    <li><a href="item.php?id=23">2456: Salad</li>
    <li><a href="item.php?id=96">4889: Cream</li>
</ul>

== foreach nest ==
<hr />
  phone: 1<br />
  fax: 2<br />
  cell: 3<br />
<hr />
  phone: 555-4444<br />
  fax: 555-3333<br />
  cell: 760-1234<br />
