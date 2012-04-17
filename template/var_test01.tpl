== $foo ==
<{$foo}>


== $ary[i] ==
<{section name="i" loop="$ary"}><{$ary[i]}>,<{/section}>
<{$ary.4}>


== $ary2[i].foo.bar ==
<{section name="i" loop="$ary2"}><{$ary2[i].foo.bar}>,<{/section}>


== $ary3[i].hoge[j].foo.bar ==
<{section name="i" loop="$ary3"}><{section name="j" loop="$ary3[i].hoge"}>
[<{$simplate.section.i.index}>,<{$simplate.section.j.index}>]<{$ary3[i].hoge[j].foo.bar}>,
<{/section}><{/section}>

== $ary4[i]->hoge ==
<{section name="i" loop="$ary4"}><{$ary4[i]->hoge}>,<{/section}>


== $ary5[i]->hoge() ==
<{section name="i" loop="$ary5"}><{$ary5[i]->hoge()}><{/section}>

== $ary5[i]->hoge('str_') ==
<{section name="i" loop="$ary5"}><{$ary5[i]->hoge("huga")}><{/section}>

== $ary6->hoge ==
<{$ary6->hoge}>


== $ary7->foo->bar() ==
<{$ary7->foo->bar()}>

== $ary8->hoge->foo->bar ==
<{section name="i" loop="$ary8"}>
<{$ary8[i]->hoge->foo->bar}>

<{/section}>

== $ary9[i]->sub[j]->bar ==
<{section name="i" loop="$ary9"}><{section name="j" loop="$ary9[i]->sub"}>(<{$simplate.section.i.index}>,<{$simplate.section.j.index}>)<{$ary9[i]->sub[j]->bar}><{/section}><{/section}>

== $ary10.foo->bar ==
<{$ary10.foo->bar}>


== $ary11->foo ==
<{$ary11->foo}>


== $ary12->hoge($foo) ==
<{$ary12->hoge($foo)}>

== $ary13.2.1 ==
<{$ary13.2.1}>
