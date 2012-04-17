== simple ==
<{* この例は $custid 配列のすべての値を表示します *}>
<{section name=customer loop=$custid}>
  id: <{$custid[customer]}><br />
<{/section}>

== section name ==
<{section name=anything loop=$myArray}>
  <{$myArray[anything].foo}><br />
  <{$section_name[anything]}><br />
  <{$addr[anything].bar}><br />
<{/section}>

== section loop ==
<{section name=customer loop=$contacts}>
<p>
  name: <{$contacts[customer].name}><br />
  home: <{$contacts[customer].home}><br />
  cell: <{$contacts[customer].cell}><br />
  e-mail: <{$contacts[customer].email}><br />
</p>
<{/section}>

== loop variable ==
<{section name=customer loop=$custid}>
<p>
  id: <{$custid[customer]}><br />
  name: <{$name[customer]}><br />
  address: <{$address[customer]}><br />
</p>
<{/section}>

== nest ==
<{section name=customer loop=$custid}>
<hr>
  id: <{$custid[customer]}><br />
  name: <{$name[customer]}><br />
  address: <{$address[customer]}><br />
<{section name=contact loop=$contact_type[customer]}>
    <{$contact_type[customer][contact]}>: <{$contact_info[customer][contact]}><br />
<{/section}>
<{/section}>

== section index ==
<{section name=customer loop=$custid}>
  <{$simplate.section.customer.index}> id: <{$custid[customer]}><br />
<{/section}>

== first last ==
<{section name=customer loop=$contacts}>
<{if $simplate.section.customer.first}>
<table>
<tr><th>id</th><th>customer</th></tr>
<{/if}>
<tr>
  <td><{$contacts[customer].name}></td>
  <td><{$contacts[customer].email}></td>
</tr>
<{if $simplate.section.customer.last}>
<tr><td></td><td><{$simplate.section.contacts.count}> customers</td></tr>
</table>
<{/if}>
<{/section}>
