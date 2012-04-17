Date: <{ $date }><br />
Author: <{$author}><br />
<{$url.id}><br />
<{ $contacts.phone.home }><br />
<{* Comments *}>
<{* More Comments
<{php}>var_dump($this);<{/php}>
*}>

<h3>== Loop ==</h3>
<table>
<{section name=i loop=$ary }>
<{if $recent_images[i].href }>
<{else if $recent_images[i].href==10 }>
<{/if}>
<{$recent_images[i].href+1}>
<{$recent_images[i].href.hoge}>
<tr>
  <td><{if $smarty.section.i.index%2==0}><{else}><{/if}></td>
  <td><{$smarty.section.i.index}></td>
  <td><{$smarty.section.i.index+1}></td>
  <td><{$ary[i]}></td>
</tr>
<{/section}>
</table>
Count:<{$smarty.section.ary.count}>
<h3>== PHP ==</h3>
<{*<{php}>var_dump($this);<{/php}>*}>

<{literal}>

function test(){}

<{/literal}>
</body>
</html>


