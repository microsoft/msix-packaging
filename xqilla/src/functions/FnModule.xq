(:
 : Copyright (c) 2004-2009
 :     Oracle. All rights reserved.
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 :     http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
 :)

module namespace fn = "http://www.w3.org/2005/xpath-functions";

declare function data($a as item()*) as xs:anyAtomicType*
{
  $a
};

(:----------------------------------------------------------------------------------------------------:)
(: String functions :)

declare function string-join($seq as xs:string*) as xs:string
{
  string-join($seq, "")
};

declare function string-join($seq as xs:string*, $join as xs:string) as xs:string
{
  if(empty($seq)) then ""
  else string-join-helper($seq, $join)
};

declare private function string-join-helper($seq as xs:string*, $join as xs:string) as xs:string
{
  if(empty(tail($seq))) then head($seq)
  else concat(head($seq), $join, string-join-helper(tail($seq), $join))
};

declare function substring-before($arg1 as xs:string?, $arg2 as xs:string?) as xs:string
{
  substring-before($arg1, $arg2, default-collation())
};

declare function substring-before($arg1 as xs:string?, $arg2 as xs:string?, $collation as xs:string)
  as xs:string
{
  let $arg1 := if(empty($arg1)) then "" else $arg1
  let $arg2 := if(empty($arg2)) then "" else $arg2
  let $arg2len := string-length($arg2)
  return
    substring($arg1, 1,
      string-index-of($arg1, 1, 1 + string-length($arg1) - $arg2len, $arg2, $arg2len, $collation) - 1)
};

declare function substring-after($arg1 as xs:string?, $arg2 as xs:string?) as xs:string
{
  substring-after($arg1, $arg2, default-collation())
};

declare function substring-after($arg1 as xs:string?, $arg2 as xs:string?, $collation as xs:string)
  as xs:string
{
  let $arg1 := if(empty($arg1)) then "" else $arg1
  let $arg2 := if(empty($arg2)) then "" else $arg2
  let $arg2len := string-length($arg2)
  let $index := string-index-of($arg1, 1, 1 + string-length($arg1) - $arg2len, $arg2, $arg2len, $collation)
  return
    if($index eq 0) then "" else substring($arg1, $index + $arg2len)
};

declare private function string-index-of($str as xs:string, $index as xs:decimal, $endindex as xs:decimal,
  $tofind as xs:string, $tofindlen as xs:decimal, $collation as xs:string) as xs:decimal
{
  if($index gt $endindex) then 0
  else if(compare($tofind, substring($str, $index, $tofindlen), $collation) eq 0) then $index
  else string-index-of($str, $index + 1, $endindex, $tofind, $tofindlen, $collation)
};

declare function codepoint-equal($arg1 as xs:string?, $arg2 as xs:string?) as xs:boolean?
{
  compare($arg1, $arg2, "http://www.w3.org/2005/xpath-functions/collation/codepoint") eq 0
};

declare function translate($arg as xs:string?, $mapString as xs:string, $transString as xs:string) as xs:string
{
  codepoints-to-string(
    let $in := string-to-codepoints($arg)
    let $map := string-to-codepoints($mapString)
    let $trans := string-to-codepoints($transString)
    for $c in $in
    let $index := head(index-of($map, $c))
    return
      if(empty($index)) then $c else $trans[$index]
  )
};

declare private function codepoint-in-utf8($c as xs:integer) as xs:integer+
{
  if($c < 128) then $c
  else if($c < 2048) then (
    ($c idiv 64) + 192,
    ($c mod 64) + 128
  )
  else if($c < 65536) then (
    ($c idiv 4096) + 224,
    (($c idiv 64) mod 64) + 128,
    ($c mod 64) + 128
  )
  else (
    ($c idiv 262144) + 240,
    (($c idiv 4096) mod 64) + 128,
    (($c idiv 64) mod 64) + 128,
    ($c mod 64) + 128
  )
};

declare private function percent-encode($c as xs:integer) as xs:integer+
{
  (: Codepoint for "%" :)
  let $percent := 37
  (: Codepoints for "0" - "9", "A" - "F" :)
  let $hex := (48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70)
  for $byte in codepoint-in-utf8($c)
  return (
    $percent,
    $hex[($byte idiv 16) + 1],
    $hex[($byte mod 16) + 1]
  )
};

declare function encode-for-uri($uri-part as xs:string?) as xs:string
{
  codepoints-to-string(
    let $A := string-to-codepoints("A")
    let $Z := string-to-codepoints("Z")
    let $a := string-to-codepoints("a")
    let $z := string-to-codepoints("z")
    let $d0 := string-to-codepoints("0")
    let $d9 := string-to-codepoints("9")
    let $hyphen := string-to-codepoints("-")
    let $underscore := string-to-codepoints("_")
    let $fullstop := string-to-codepoints(".")
    let $tilde := string-to-codepoints("~")

    for $c in string-to-codepoints($uri-part)
    return
      if(($c ge $A and $c le $Z) or
        ($c ge $a and $c le $z) or
        ($c ge $d0 and $c le $d9) or
        $c eq $hyphen or $c eq $underscore or
        $c eq $fullstop or $c eq $tilde)
      then $c
      else percent-encode($c)
  )
};

declare function iri-to-uri($iri as xs:string?) as xs:string
{
  codepoints-to-string(
    let $leftangle := string-to-codepoints("<")
    let $rightangle := string-to-codepoints(">")
    let $doublequote := string-to-codepoints('"')
    let $space := string-to-codepoints(" ")
    let $leftcurly := string-to-codepoints("{")
    let $rightcurly := string-to-codepoints("}")
    let $bar := string-to-codepoints("|")
    let $backslash := 92
    let $hat := string-to-codepoints("^")
    let $backtick := string-to-codepoints("`")

    for $c in string-to-codepoints($iri)
    return
      if($c lt 32 or $c gt 126 or
        $c eq $leftangle or $c eq $rightangle or
        $c eq $doublequote or $c eq $space or
        $c eq $leftcurly or $c eq $rightcurly or
        $c eq $bar or $c eq $backslash or
        $c eq $hat or $c eq $backtick)
      then percent-encode($c)
      else $c
  )
};

declare function escape-html-uri($iri as xs:string?) as xs:string
{
  codepoints-to-string(
    for $c in string-to-codepoints($iri)
    return
      if($c lt 32 or $c gt 126)
      then percent-encode($c)
      else $c
  )
};

(:----------------------------------------------------------------------------------------------------:)
(: QName functions :)

declare function resolve-QName($qname as xs:string?, $element as element()) as xs:QName?
{
  typeswitch($qname)
  case $qname as xs:string return
    let $mock := QName("dummy", $qname)
    let $uri := namespace-uri-for-prefix(prefix-from-QName($mock), $element)
    return
      if(empty(prefix-from-QName($mock)) or exists($uri)) then QName($uri, $qname)
      else error(xs:QName("err:FONS0004"), concat("No namespace found for prefix: """, prefix-from-QName($mock), """"))
  default return ()
};

declare function namespace-uri-for-prefix($prefix as xs:string?, $element as element()) as xs:anyURI?
{
  let $prefix := if(empty($prefix)) then "" else $prefix
  return $element/namespace::*[name() eq $prefix]/xs:anyURI(.)
};

declare function in-scope-prefixes($element as element()) as xs:string*
{
  $element/namespace::*/name()
};

(:----------------------------------------------------------------------------------------------------:)
(: Boolean functions :)

declare function true() as xs:boolean
{
  xs:boolean("1")
};

declare function false() as xs:boolean
{
  xs:boolean("0")
};

declare function boolean($arg as item()*) as xs:boolean
{
  $arg and true()
};

declare function exists($arg as item()*) as xs:boolean
{
  not(empty($arg))
};

(:----------------------------------------------------------------------------------------------------:)
(: Sequence functions :)

declare function zero-or-one($arg as item()*) as item()?
{
  typeswitch($arg)
  case $arg_ as item()? return $arg_
  default return error(xs:QName("err:FORG0003"), "Sequence contains more then one item")
};

declare function one-or-more($arg as item()*) as item()+
{
  typeswitch($arg)
  case $arg_ as item()+ return $arg_
  default return error(xs:QName("err:FORG0004"), "Sequence is empty")
};

declare function exactly-one($arg as item()*) as item()
{
  typeswitch($arg)
  case $arg_ as item() return $arg_
  case empty-sequence() return error(xs:QName("err:FORG0005"), "Sequence is empty")
  default return error(xs:QName("err:FORG0005"), "Sequence contains more then one item")
};

declare function index-of($seq as xs:anyAtomicType*, $search as xs:anyAtomicType) as xs:integer*
{
  (: Check for NaN :)
  if($search ne $search) then () else index-of-helper($seq, $search, default-collation())
};

declare function index-of($seq as xs:anyAtomicType*, $search as xs:anyAtomicType,
   $collation as xs:string) as xs:integer*
{
  (: Check for NaN :)
  if($search ne $search) then () else index-of-helper($seq, $search, $collation)
};

declare private function index-of-helper($seq as xs:anyAtomicType*, $search as xs:anyAtomicType,
  $collation as xs:string) as xs:integer*
{
  for $s at $p in $seq
  where empty(tail(distinct-values(($s, $search), $collation)))
  return $p
};

declare function insert-before($target as item()*, $position as xs:integer, $inserts as item()*)
  as item()*
{
  if($position le 1 or empty($inserts)) then ($inserts, $target)
  else (head($target), insert-before(tail($target), $position - 1, $inserts))
};

declare function remove($target as item()*, $position as xs:integer) as item()*
{
  $target[position() ne $position]
};

declare function reverse($seq as item()*) as item()*
{
  if(empty($seq)) then ()
  else (reverse(tail($seq)), head($seq))
};

declare function subsequence($sourceSeq as item()*, $startingLoc as xs:double) as item()*
{
  let $s := round($startingLoc)
  return
    if($s < 1) then $sourceSeq
    else xqilla:drop($sourceSeq, $s - 1)
};

declare function subsequence($sourceSeq as item()*, $startingLoc as xs:double,
  $length as xs:double) as item()*
{
  let $s := round($startingLoc)
  return
    if($s < 1) then take($sourceSeq, round($length) + $s - 1)
    else take(xqilla:drop($sourceSeq, $s - 1), round($length))
};

declare private function take($seq as item()*, $num as xs:double) as item()*
{
  if($num le 0 or empty($seq)) then ()
  else (head($seq), take(tail($seq), $num - 1))
};

declare function deep-equal($parameter1 as item()*, $parameter2 as item()*) as xs:boolean
{
  deep-equal($parameter1, $parameter2, default-collation())
};

declare function deep-equal($p1 as item()*, $p2 as item()*, $collation as xs:string) as xs:boolean
{
  if(empty($p1)) then empty($p2) else
  if(empty($p2)) then false() else

  let $i1 := head($p1), $i2 := head($p2)
  return
    typeswitch($i1)

    case $a1 as xs:anyAtomicType return
      typeswitch($i2)
      case $a2 as xs:anyAtomicType return
        empty(tail(distinct-values(($a1, $a2), $collation))) and
        deep-equal(tail($p1), tail($p2), $collation)
      case node() return false()
      default return deep-equal-error()

    case $n1 as node() return (
      typeswitch($i2)
      case xs:anyAtomicType return false()
      case $n2 as node() return (
        deep-equal-nodes($n1, $n2, $collation) and
        deep-equal(tail($p1), tail($p2), $collation)
      )
      default return deep-equal-error()
    )

    default return deep-equal-error()
};

declare private function deep-equal-error()
{
  error(xs:QName("err:FOTY0015"), "An argument to fn:deep-equal() contains a function item")
};

declare private function deep-equal-nodes($p1 as node()*, $p2 as node()*, $collation as xs:string) as xs:boolean
{
  if(empty($p1)) then empty($p2) else
  if(empty($p2)) then false() else

  let $i1 := head($p1), $i2 := head($p2)
  return
  typeswitch($i1)

  case document-node() return
    typeswitch($i2)
    case document-node() return
      deep-equal-nodes($i1/(*|text()), $i2/(*|text()), $collation) and
      deep-equal-nodes(tail($p1), tail($p2), $collation)
    default return false()

  case text() return
    typeswitch($i2)
    case text() return
      empty(tail(distinct-values(($i1, $i2), $collation))) and
      deep-equal-nodes(tail($p1), tail($p2), $collation)
    default return false()

  case comment() return
    typeswitch($i2)
    case comment() return
      empty(tail(distinct-values(($i1, $i2), $collation))) and
      deep-equal-nodes(tail($p1), tail($p2), $collation)
    default return false()

  case processing-instruction() return
    typeswitch($i2)
    case processing-instruction() return
      node-name($i1) eq node-name($i2) and
      empty(tail(distinct-values(($i1, $i2), $collation))) and
      deep-equal-nodes(tail($p1), tail($p2), $collation)
    default return false()

  case attribute() return
    typeswitch($i2)
    case attribute() return
      node-name($i1) eq node-name($i2) and
      deep-equal(data($i1), data($i2), $collation) and
      deep-equal-nodes(tail($p1), tail($p2), $collation)
    default return false()

  case element() return
    typeswitch($i2)
    case element() return (
      node-name($i1) eq node-name($i2) and
      count($i1/@*) eq count($i2/@*) and
      (every $a1 in $i1/@* satisfies
        some $a2 in $i2/@* satisfies deep-equal-nodes($a1, $a2, $collation)) and

      (if(empty($i1/*) and empty($i2/*))
        then deep-equal(data($i1), data($i2), $collation)
        else deep-equal-nodes($i1/(*|text()), $i2/(*|text()), $collation)) and

      deep-equal-nodes(tail($p1), tail($p2), $collation)
    )
    default return false()

  
  default (: namespace-node() :) return
    typeswitch($i2)
    case namespace-node() return
      deep-equal(node-name($i1), node-name($i2), $collation) and
      codepoint-equal($i1, $i2) and
      deep-equal-nodes(tail($p1), tail($p2), $collation)
    default return false()
};

(:----------------------------------------------------------------------------------------------------:)
(: Aggregate functions :)

declare function avg($arg as xs:anyAtomicType*) as xs:anyAtomicType?
{
  sum($arg, ()) div count($arg)
};

declare function sum($arg as xs:anyAtomicType*) as xs:anyAtomicType
{
  sum($arg, 0)
};

declare function sum($arg as xs:anyAtomicType*, $zero as xs:anyAtomicType?) as xs:anyAtomicType?
{
  typeswitch(head($arg))
  case empty-sequence() return $zero
  case $head as xs:untypedAtomic return numeric-sum(tail($arg), xs:double($head))
  case $head as xs:double | xs:float | xs:decimal return
    if($head ne $head) then $head else numeric-sum(tail($arg), $head)
  case $head as xs:yearMonthDuration return yearMonthDuration-sum(tail($arg), $head)
  case $head as xs:dayTimeDuration return dayTimeDuration-sum(tail($arg), $head)
  default return error(xs:QName("err:FORG0006"), "Invalid argument to fn:sum() function")
};

declare private function numeric-sum($arg as xs:anyAtomicType*, $result as xs:anyAtomicType) as xs:anyAtomicType
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:untypedAtomic return numeric-sum(tail($arg), xs:double($head) + $result)
  case $head as xs:double | xs:float | xs:decimal return
    if($head ne $head) then $head else numeric-sum(tail($arg), $head + $result)
  default return error(xs:QName("err:FORG0006"), "Invalid argument to fn:sum() function")
};

declare private function yearMonthDuration-sum($arg as xs:anyAtomicType*, $result as xs:yearMonthDuration) as xs:yearMonthDuration
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:yearMonthDuration return yearMonthDuration-sum(tail($arg), $head + $result)
  default return error(xs:QName("err:FORG0006"), "Invalid argument to fn:sum() function")
};

declare private function dayTimeDuration-sum($arg as xs:anyAtomicType*, $result as xs:dayTimeDuration) as xs:dayTimeDuration
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:dayTimeDuration return dayTimeDuration-sum(tail($arg), $head + $result)
  default return error(xs:QName("err:FORG0006"), "Invalid argument to fn:sum() function")
};

declare function max($arg as xs:anyAtomicType*) as xs:anyAtomicType?
{
  max($arg, default-collation())
};

declare function max($arg as xs:anyAtomicType*, $collation as xs:string) as xs:anyAtomicType?
{
  typeswitch(head($arg))
  case empty-sequence() return ()
  case $head as xs:untypedAtomic | xs:double return double-max(tail($arg), xs:double($head))
  case $head as xs:float return float-max(tail($arg), $head)
  case $head as xs:decimal return decimal-max(tail($arg), $head)
  case $head as xs:anyURI | xs:string return string-max(tail($arg), xs:string($head), $collation)
  case $head as xs:boolean return boolean-max(tail($arg), $head)
  case $head as xs:date return date-max(tail($arg), $head)
  case $head as xs:time return time-max(tail($arg), $head)
  case $head as xs:dateTime return dateTime-max(tail($arg), $head)
  case $head as xs:yearMonthDuration return yearMonthDuration-max(tail($arg), $head)
  case $head as xs:dayTimeDuration return dayTimeDuration-max(tail($arg), $head)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function double-max($arg as xs:anyAtomicType*, $result as xs:double) as xs:double
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:untypedAtomic | xs:double | xs:float | xs:decimal return
    let $head := xs:double($head) return
      double-max(tail($arg), if($head gt $result or $head ne $head) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function float-max($arg as xs:anyAtomicType*, $result as xs:float) as xs:anyAtomicType
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:untypedAtomic | xs:double return double-max($arg, xs:double($result))
  case $head as xs:float | xs:decimal return
    let $head := xs:float($head) return
      float-max(tail($arg), if($head gt $result or $head ne $head) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function decimal-max($arg as xs:anyAtomicType*, $result as xs:decimal) as xs:anyAtomicType
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:untypedAtomic | xs:double return double-max($arg, xs:double($result))
  case $head as xs:float return float-max($arg, xs:float($result))
  case $head as xs:decimal return decimal-max(tail($arg), if($head gt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function string-max($arg as xs:anyAtomicType*, $result as xs:string, $collation as xs:string) as xs:string
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:string | xs:anyURI return
    string-max(tail($arg), if(compare($head, $result, $collation) gt 0) then xs:string($head) else $result, $collation)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function boolean-max($arg as xs:anyAtomicType*, $result as xs:boolean) as xs:boolean
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:boolean return boolean-max(tail($arg), if($head gt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function date-max($arg as xs:anyAtomicType*, $result as xs:date) as xs:date
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:date return date-max(tail($arg), if($head gt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function time-max($arg as xs:anyAtomicType*, $result as xs:time) as xs:time
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:time return time-max(tail($arg), if($head gt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function dateTime-max($arg as xs:anyAtomicType*, $result as xs:dateTime) as xs:dateTime
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:dateTime return dateTime-max(tail($arg), if($head gt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function yearMonthDuration-max($arg as xs:anyAtomicType*, $result as xs:yearMonthDuration) as xs:yearMonthDuration
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:yearMonthDuration return yearMonthDuration-max(tail($arg), if($head gt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare private function dayTimeDuration-max($arg as xs:anyAtomicType*, $result as xs:dayTimeDuration) as xs:dayTimeDuration
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:dayTimeDuration return dayTimeDuration-max(tail($arg), if($head gt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:max() function")
};

declare function min($arg as xs:anyAtomicType*) as xs:anyAtomicType?
{
  min($arg, default-collation())
};

declare function min($arg as xs:anyAtomicType*, $collation as xs:string) as xs:anyAtomicType?
{
  typeswitch(head($arg))
  case empty-sequence() return ()
  case $head as xs:untypedAtomic | xs:double return double-min(tail($arg), xs:double($head))
  case $head as xs:float return float-min(tail($arg), $head)
  case $head as xs:decimal return decimal-min(tail($arg), $head)
  case $head as xs:anyURI | xs:string return string-min(tail($arg), xs:string($head), $collation)
  case $head as xs:boolean return boolean-min(tail($arg), $head)
  case $head as xs:date return date-min(tail($arg), $head)
  case $head as xs:time return time-min(tail($arg), $head)
  case $head as xs:dateTime return dateTime-min(tail($arg), $head)
  case $head as xs:yearMonthDuration return yearMonthDuration-min(tail($arg), $head)
  case $head as xs:dayTimeDuration return dayTimeDuration-min(tail($arg), $head)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function double-min($arg as xs:anyAtomicType*, $result as xs:double) as xs:double
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:untypedAtomic | xs:double | xs:float | xs:decimal return
    let $head := xs:double($head) return
      double-min(tail($arg), if($head lt $result or $head ne $head) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function float-min($arg as xs:anyAtomicType*, $result as xs:float) as xs:anyAtomicType
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:untypedAtomic | xs:double return double-min($arg, xs:double($result))
  case $head as xs:float | xs:decimal return
    let $head := xs:float($head) return
      float-min(tail($arg), if($head lt $result or $head ne $head) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function decimal-min($arg as xs:anyAtomicType*, $result as xs:decimal) as xs:anyAtomicType
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:untypedAtomic | xs:double return double-min($arg, xs:double($result))
  case $head as xs:float return float-min($arg, xs:float($result))
  case $head as xs:decimal return decimal-min(tail($arg), if($head lt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function string-min($arg as xs:anyAtomicType*, $result as xs:string, $collation as xs:string) as xs:string
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:string | xs:anyURI return
    string-min(tail($arg), if(compare($head, $result, $collation) lt 0) then xs:string($head) else $result, $collation)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function boolean-min($arg as xs:anyAtomicType*, $result as xs:boolean) as xs:boolean
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:boolean return boolean-min(tail($arg), if($head lt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function date-min($arg as xs:anyAtomicType*, $result as xs:date) as xs:date
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:date return date-min(tail($arg), if($head lt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function time-min($arg as xs:anyAtomicType*, $result as xs:time) as xs:time
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:time return time-min(tail($arg), if($head lt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function dateTime-min($arg as xs:anyAtomicType*, $result as xs:dateTime) as xs:dateTime
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:dateTime return dateTime-min(tail($arg), if($head lt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function yearMonthDuration-min($arg as xs:anyAtomicType*, $result as xs:yearMonthDuration) as xs:yearMonthDuration
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:yearMonthDuration return yearMonthDuration-min(tail($arg), if($head lt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

declare private function dayTimeDuration-min($arg as xs:anyAtomicType*, $result as xs:dayTimeDuration) as xs:dayTimeDuration
{
  typeswitch(head($arg))
  case empty-sequence() return $result
  case $head as xs:dayTimeDuration return dayTimeDuration-min(tail($arg), if($head lt $result) then $head else $result)
  default return error(xs:QName("err:FORG0006"), "Uncomparable items in argument to fn:min() function")
};

(:----------------------------------------------------------------------------------------------------:)
(: XQuery 1.1 functions :)

(: TBD These should probably only be imported if we're parsing XQuery 1.1 - jpcs :)

declare function map($f as function(item()) as item()*, $seq as item()*) as item()*
{
  if(empty($seq)) then ()
  else ($f(head($seq)), map($f, tail($seq)))
};

declare function filter($f as function(item()) as xs:boolean, $seq as item()*) as item()*
{
  if(empty($seq)) then ()
  else (
    if($f(head($seq))) then head($seq) else (),
    filter($f, tail($seq))
  )
};

declare function fold-left($f as function(item()*, item()) as item()*, $zero as item()*,
  $seq as item()*) as item()*
{
  if(empty($seq)) then $zero
  else fold-left($f, $f($zero, head($seq)), tail($seq))
};

declare function fold-right($f as function(item(), item()*) as item()*, $zero as item()*,
  $seq as item()*) as item()*
{
  if(empty($seq)) then $zero
  else $f(head($seq), fold-right($f, $zero, tail($seq)))
};

declare function map-pairs($f as function(item(), item()) as item()*, $seq1 as item()*,
  $seq2 as item()*) as item()*
{
   if(empty($seq1) or empty($seq2)) then ()
   else (
     $f(head($seq1), head($seq2)),
     map-pairs($f, tail($seq1), tail($seq2))
   )
};

