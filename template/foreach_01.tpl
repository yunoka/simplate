== item ==
<ul>
<{foreach from=$myArray item=foo}>
    <li><{$foo}></li>
<{/foreach}>
</ul>

== item & key ==
<ul>
<{foreach from=$myArray2 key=k item=v}>
    <li><{$k}>: <{$v}></li>
<{/foreach}>
</ul>

== item list ==
<ul>
<{foreach from=$items key=myId item=i}>
    <li><a href="item.php?id=<{$myId}>"><{$i.no}>: <{$i.label}></li>
<{/foreach}>
</ul>

== foreach nest ==
<{foreach item=contact from=$contacts}>
<hr />
<{foreach key=key item=item from=$contact}>
  <{$key}>: <{$item}><br />
<{/foreach}>
<{/foreach}>
