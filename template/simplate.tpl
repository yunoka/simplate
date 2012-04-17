<html><head>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<title>My <{$myname}></title><style type="text/css">
<!--
pre{ background-color:#F0F8FF; }
// -->
</style></head><body>
<h3>== assign ==</h3>
Date:<{$date}><br>
<{literal}>
<{php}> can be described within literal tag
<{/literal}><h3>== if statement ==</h3>
<{if $greeting=='hello' || $greeting=="bye" }>
Hello
<{else if $greet=="hoge"}>
Hoge1
<{elseif $greet=="hoge"}>
Hoge2
<{else}>
Bye
<{/if}>

<{section name=i loop=$entry}>
A
  <{section name=j loop=$entry[i]}>
  B
  <{$entry[i].comments[j].comment_id}>
  <{/section}>
<{/section}>

<h3>== section statement ==</h3>
<table>
<{section name=i loop=$ary }>
<tr><td><{$ary[i]}></td></tr>
<tr><td><{$ary[i]+1}></td></tr>
<{/section}>
</table>

<h3>== include statement ==</h3>
<{include file="relative_file.tpl"}>

<{$fetched_content}>

<h3>== foreach ==</h3>
<{foreach item=curr_id from=$custid}>
  id: <{$curr_id}><br />
<{/foreach}>
<hr />
<{foreach key=key item=item from=$contacts}>
<{$key}>: <{$item}><br />
<{/foreach}>

<{foreach key=key item=item from=$ary2}>
<{$key}>: <{$item}><br />
<{/foreach}>

<h3>== var_dump ==</h3>
A<{*Single line comment *}>B
<{php}>echo date('Y');<{/php}>
<{*
<{php}>var_dump($this);<{/php}>
<h3>== TODO ==</h3>
<ul>
<li><strike>Compiled once with file timestamp.</strike></li>
<li>Support <b>include</b> method.</li>
<li>Support cached static templates.</li>
<li>Support change parameters without compile.</li>
</ul>
*}>A<{*
<h3>== TODO ==</h3>
<ul>
<li><strike>Compiled once with file timestamp.</strike></li>
<li>Support <b>include</b> method.</li>
<li>Support cached static templates.</li>
<li>Support change parameters without compile.</li>
</ul>
*}>
<!-- prefilter remove this -->
</body>
</html>
