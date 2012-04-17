--TEST--
Check for Simplate Values
--FILE--
<?php

class Klass
{
    public $foo = 'variable test';
    function hoge($str = "test")
    {
        return "Klass::hoge($str)\n";
    }
    function bar()
    {
        return "Klass::bar()\n";
    }
}

$simplate = new Simplate();
///////////////////////////////////////
//$simplate->force_compile = true;
///////////////////////////////////////

// $foo
$simplate->assign('foo', "foo1");

// $ary[i]
$simplate->assign('ary', range("a","e"));

// $ary[i].foo.bar
$ary2 = array();
for ($i = 0; $i < 3; $i++) {
    $ary2[] = array('foo' => array('bar' => "foo_bar_".$i));
}
$simplate->assign('ary2', $ary2);

// $ary3[i].ary2[j].foo
$ary3 = array();
for ($i = 0; $i < 4; $i++) {
    $ary3[] = array('hoge' => $ary2);
}
//print_r($ary3);
$simplate->assign('ary3', $ary3);

// $ary4[i]->hoge
$simplate->assign('ary4', array((object)array('hoge' => "huga"), (object)array('hoge' => "huga2")));

// $ary5->hoge()
// $ary5->hoge('str_')
$ary5 = array();
for ($i = 0; $i < 3; $i++) {
    $ary5[] = new Klass();
}
$simplate->assign('ary5', $ary5);

// $ary6->hoge
$simplate->assign('ary6', (object)array('hoge' => "stdClass::hoge"));

// $ary7->foo->bar()
$simplate->assign('ary7', (object)array('foo' => new Klass()));

// $ary8[i]->hoge->foo->bar
$ary8 = array();
for ($i = 0; $i < 4; $i++) {
    $ary8[] = (object)array('hoge' => (object)array('foo' => (object)array('bar' => "hoge::foo::bar($i)")));
}
$simplate->assign('ary8', $ary8);

// $ary9[i]->sub[j]->bar
$ary9 = array();
for ($i = 0; $i < 3; $i++) {
    $o = new Klass();
    for ($j = 0; $j < 2; $j++) {
        $o->sub[] = (object)array('bar' => "bar\n");
    }
    $ary9[] = $o;
}
$simplate->assign('ary9', $ary9);

// $ary10.foo->bar
$ary10['foo'] = (object)array('bar' => 'hoge');
$simplate->assign('ary10', $ary10);

// $ary11->name
$simplate->assign('ary11', new Klass());

// $ary12->hoge($foo)
$simplate->assign('ary12', new Klass());

// $ary13.2.1
$simplate->assign('ary13',
    array(
        '555-222-9876',
        'simplate@simplate.example.com',
        array(
            '555-444-3333',
            '555-111-1234'
        )
    )
);

$simplate->display("var_test01.tpl");

?>
--EXPECT--
== $foo ==
foo1

== $ary[i] ==
a,b,c,d,e,e

== $ary2[i].foo.bar ==
foo_bar_0,foo_bar_1,foo_bar_2,

== $ary3[i].hoge[j].foo.bar ==
[0,0]foo_bar_0,
[0,1]foo_bar_1,
[0,2]foo_bar_2,
[1,0]foo_bar_0,
[1,1]foo_bar_1,
[1,2]foo_bar_2,
[2,0]foo_bar_0,
[2,1]foo_bar_1,
[2,2]foo_bar_2,
[3,0]foo_bar_0,
[3,1]foo_bar_1,
[3,2]foo_bar_2,

== $ary4[i]->hoge ==
huga,huga2,

== $ary5[i]->hoge() ==
Klass::hoge(test)
Klass::hoge(test)
Klass::hoge(test)

== $ary5[i]->hoge('str_') ==
Klass::hoge(huga)
Klass::hoge(huga)
Klass::hoge(huga)

== $ary6->hoge ==
stdClass::hoge

== $ary7->foo->bar() ==
Klass::bar()

== $ary8->hoge->foo->bar ==
hoge::foo::bar(0)
hoge::foo::bar(1)
hoge::foo::bar(2)
hoge::foo::bar(3)

== $ary9[i]->sub[j]->bar ==
(0,0)bar
(0,1)bar
(1,0)bar
(1,1)bar
(2,0)bar
(2,1)bar

== $ary10.foo->bar ==
hoge

== $ary11->foo ==
variable test

== $ary12->hoge($foo) ==
Klass::hoge(foo1)

== $ary13.2.1 ==
555-111-1234
