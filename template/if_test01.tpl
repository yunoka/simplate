// if $ary9[i]->sub[j]->bar
<{section name="i" loop="$ary9"}><{section name="j" loop="$ary9[i]->sub"}>
<{if $ary9[i]->sub[j]->bar==$bar}>[found]<{/if}>
<{$ary9[i]->sub[j]->bar}>
<{/section}><{/section}>
