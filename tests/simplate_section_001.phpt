--TEST--
Check for Simplate section
--FILE--
<?php

$simplate = new Simplate();

// simple
$id = array(1000, 1001, 1002);
$simplate->assign('custid', $id);

// section name
$myArray = array(
    array('foo' => 'foo1'),
    array('foo' => 'foo2'),
    array('foo' => 'foo3')
    );
$section_name = array('name1', 'name2', 'name3');
$address = array(
    array('bar' => '1-11-1'),
    array('bar' => '2-22-2'),
    array('bar' => '3-33-3')
    );
$simplate->assign('myArray', $myArray);
$simplate->assign('section_name', $section_name);
$simplate->assign('addr', $address);

// section loop
$data2 = array(
    array('name' => 'Taro Yamada', 'home' => '555-555-5555',
        'cell' => '666-555-5555', 'email' => 'yamada@simplate.com'),
    array('name' => 'Hanako Tanaka', 'home' => '777-555-5555',
        'cell' => '888-555-5555', 'email' => 'tanaka@simplate.com'),
    array('name' => 'Ichiro Suzuki', 'home' => '000-555-5555',
        'cell' => '123456', 'email' => 'suzuki@simplate.com')
        );
$simplate->assign('contacts', $data2);

// loop variable
$fullnames = array('Taro Yamada', 'Hanako Tanaka', 'Ichiro Suzuki');
$addr = array('253 Abbey road', '417 Mulberry ln', '5605 apple st');
$simplate->assign('name', $fullnames);
$simplate->assign('address', $addr);

// nest
$types = array(
           array('home phone', 'cell phone', 'e-mail'),
           array('home phone', 'web'),
           array('cell phone')
         );
$simplate->assign('contact_type', $types);

$info = array(
           array('555-555-5555', '666-555-5555', 'yamada@simplate.com'),
           array('123-456-4', 'www.simplate.com'),
           array('0457878')
        );
$simplate->assign('contact_info', $info);

$simplate->display("section_01.tpl");

?>
--EXPECT--
== simple ==

  id: 1000<br />
  id: 1001<br />
  id: 1002<br />

== section name ==
  foo1<br />
  name1<br />
  1-11-1<br />
  foo2<br />
  name2<br />
  2-22-2<br />
  foo3<br />
  name3<br />
  3-33-3<br />

== section loop ==
<p>
  name: Taro Yamada<br />
  home: 555-555-5555<br />
  cell: 666-555-5555<br />
  e-mail: yamada@simplate.com<br />
</p>
<p>
  name: Hanako Tanaka<br />
  home: 777-555-5555<br />
  cell: 888-555-5555<br />
  e-mail: tanaka@simplate.com<br />
</p>
<p>
  name: Ichiro Suzuki<br />
  home: 000-555-5555<br />
  cell: 123456<br />
  e-mail: suzuki@simplate.com<br />
</p>

== loop variable ==
<p>
  id: 1000<br />
  name: Taro Yamada<br />
  address: 253 Abbey road<br />
</p>
<p>
  id: 1001<br />
  name: Hanako Tanaka<br />
  address: 417 Mulberry ln<br />
</p>
<p>
  id: 1002<br />
  name: Ichiro Suzuki<br />
  address: 5605 apple st<br />
</p>

== nest ==
<hr>
  id: 1000<br />
  name: Taro Yamada<br />
  address: 253 Abbey road<br />
    home phone: 555-555-5555<br />
    cell phone: 666-555-5555<br />
    e-mail: yamada@simplate.com<br />
<hr>
  id: 1001<br />
  name: Hanako Tanaka<br />
  address: 417 Mulberry ln<br />
    home phone: 123-456-4<br />
    web: www.simplate.com<br />
<hr>
  id: 1002<br />
  name: Ichiro Suzuki<br />
  address: 5605 apple st<br />
    cell phone: 0457878<br />

== section index ==
  0 id: 1000<br />
  1 id: 1001<br />
  2 id: 1002<br />

== first last ==
<table>
<tr><th>id</th><th>customer</th></tr>
<tr>
  <td>Taro Yamada</td>
  <td>yamada@simplate.com</td>
</tr>
<tr>
  <td>Hanako Tanaka</td>
  <td>tanaka@simplate.com</td>
</tr>
<tr>
  <td>Ichiro Suzuki</td>
  <td>suzuki@simplate.com</td>
</tr>
<tr><td></td><td>3 customers</td></tr>
</table>
